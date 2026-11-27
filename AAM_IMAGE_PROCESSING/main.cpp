#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

struct Pixel {
    uint8_t r = 0, g = 0, b = 0;
    bool operator==(const Pixel& other) const {
        return r == other.r && g == other.g && b == other.b;
    }
};

struct Image {
    int width = 0, height = 0;
    std::vector<std::vector<Pixel>> matrix;
    Image() = default;
    Image(int w, int h) : width(w), height(h), matrix(h, std::vector<Pixel>(w)) {}
};

struct Region {
    int startRow, endRow, startCol, endCol;
};

enum class Operation {
    BRIGHTNESS = 1,
    GRAYSCALE  = 2,
    BOX_BLUR   = 3,
    SHARPEN    = 4
};

// -------------------- Pure C++ 24-bit BMP I/O --------------------

#pragma pack(push, 1)
struct BMPHeader {
    uint16_t fileType{0x4D42};     // "BM"
    uint32_t fileSize{0};
    uint16_t reserved1{0};
    uint16_t reserved2{0};
    uint32_t offsetData{54};
    uint32_t headerSize{40};
    int32_t  width{0};
    int32_t  height{0};
    uint16_t planes{1};
    uint16_t bitCount{24};
    uint32_t compression{0};
    uint32_t imageSize{0};
    int32_t  xPixelsPerMeter{2835};
    int32_t  yPixelsPerMeter{2835};
    uint32_t colorsUsed{0};
    uint32_t colorsImportant{0};
};
#pragma pack(pop)

Image loadBMP(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot open file: " + filename);

    BMPHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (header.fileType != 0x4D42)
        throw std::runtime_error("Not a valid BMP file: " + filename);
    if (header.bitCount != 24 || header.compression != 0)
        throw std::runtime_error("Only uncompressed 24-bit BMP files are supported.");

    int w = header.width;
    int h = std::abs(header.height);
    bool topDown = (header.height < 0);

    Image img(w, h);
    int rowPadded = (w * 3 + 3) & (~3);
    std::vector<uint8_t> rowBuf(rowPadded);

    file.seekg(header.offsetData, std::ios::beg);
    for (int i = 0; i < h; ++i) {
        int r = topDown ? i : (h - 1 - i);
        file.read(reinterpret_cast<char*>(rowBuf.data()), rowPadded);
        for (int c = 0; c < w; ++c) {
            img.matrix[r][c] = {
                rowBuf[c * 3 + 2], // R
                rowBuf[c * 3 + 1], // G
                rowBuf[c * 3 + 0]  // B
            };
        }
    }
    return img;
}

void saveBMP(const std::string& filename, const Image& img) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot write file: " + filename);

    int rowPadded = (img.width * 3 + 3) & (~3);
    BMPHeader header;
    header.width = img.width;
    header.height = img.height;
    header.imageSize = rowPadded * img.height;
    header.fileSize = sizeof(BMPHeader) + header.imageSize;

    file.write(reinterpret_cast<const char*>(&header), sizeof(header));

    std::vector<uint8_t> rowBuf(rowPadded, 0);
    for (int r = img.height - 1; r >= 0; --r) {
        for (int c = 0; c < img.width; ++c) {
            rowBuf[c * 3 + 0] = img.matrix[r][c].b;
            rowBuf[c * 3 + 1] = img.matrix[r][c].g;
            rowBuf[c * 3 + 2] = img.matrix[r][c].r;
        }
        file.write(reinterpret_cast<const char*>(rowBuf.data()), rowPadded);
    }
}

Image generateSampleImage(int width, int height) {
    Image img(width, height);
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            uint8_t red   = static_cast<uint8_t>((c * 255) / width);
            uint8_t green = static_cast<uint8_t>((r * 255) / height);
            uint8_t blue  = static_cast<uint8_t>(((r + c) * 128) / (width + height));
            if (((r / 40) + (c / 40)) % 2 == 0) {
                blue = static_cast<uint8_t>(std::min(255, blue + 80));
            }
            img.matrix[r][c] = {red, green, blue};
        }
    }
    return img;
}

// -------------------- Pixel Matrix Operations --------------------

void processRegion(const Image& input, Image& output, const Region& reg,
                   Operation op, int brightnessDelta) {
    static const int sharpenKernel[3][3] = {
        { 0, -1,  0},
        {-1,  5, -1},
        { 0, -1,  0}
    };

    for (int r = reg.startRow; r < reg.endRow; ++r) {
        for (int c = reg.startCol; c < reg.endCol; ++c) {
            if (op == Operation::BRIGHTNESS) {
                const Pixel& p = input.matrix[r][c];
                output.matrix[r][c] = {
                    static_cast<uint8_t>(std::clamp(static_cast<int>(p.r) + brightnessDelta, 0, 255)),
                    static_cast<uint8_t>(std::clamp(static_cast<int>(p.g) + brightnessDelta, 0, 255)),
                    static_cast<uint8_t>(std::clamp(static_cast<int>(p.b) + brightnessDelta, 0, 255))
                };
            } else if (op == Operation::GRAYSCALE) {
                const Pixel& p = input.matrix[r][c];
                uint8_t gray = static_cast<uint8_t>(0.299 * p.r + 0.587 * p.g + 0.114 * p.b);
                output.matrix[r][c] = {gray, gray, gray};
            } else if (op == Operation::BOX_BLUR) {
                int sumR = 0, sumG = 0, sumB = 0;
                for (int kr = -1; kr <= 1; ++kr) {
                    int nr = std::clamp(r + kr, 0, input.height - 1);
                    for (int kc = -1; kc <= 1; ++kc) {
                        int nc = std::clamp(c + kc, 0, input.width - 1);
                        sumR += input.matrix[nr][nc].r;
                        sumG += input.matrix[nr][nc].g;
                        sumB += input.matrix[nr][nc].b;
                    }
                }
                output.matrix[r][c] = {
                    static_cast<uint8_t>(sumR / 9),
                    static_cast<uint8_t>(sumG / 9),
                    static_cast<uint8_t>(sumB / 9)
                };
            } else if (op == Operation::SHARPEN) {
                int sumR = 0, sumG = 0, sumB = 0;
                for (int kr = -1; kr <= 1; ++kr) {
                    int nr = std::clamp(r + kr, 0, input.height - 1);
                    for (int kc = -1; kc <= 1; ++kc) {
                        int nc = std::clamp(c + kc, 0, input.width - 1);
                        int w = sharpenKernel[kr + 1][kc + 1];
                        sumR += input.matrix[nr][nc].r * w;
                        sumG += input.matrix[nr][nc].g * w;
                        sumB += input.matrix[nr][nc].b * w;
                    }
                }
                output.matrix[r][c] = {
                    static_cast<uint8_t>(std::clamp(sumR, 0, 255)),
                    static_cast<uint8_t>(std::clamp(sumG, 0, 255)),
                    static_cast<uint8_t>(std::clamp(sumB, 0, 255))
                };
            }
        }
    }
}

// -------------------- Parallel Divide & Conquer --------------------

void divideAndConquerParallel(const Image& input, Image& output, const Region& reg,
                              Operation op, int brightnessDelta,
                              int depth, int maxParallelDepth, int minBlockSize) {
    int rows = reg.endRow - reg.startRow;
    int cols = reg.endCol - reg.startCol;
    if (rows <= 0 || cols <= 0) return;

    if (rows <= minBlockSize || cols <= minBlockSize) {
        processRegion(input, output, reg, op, brightnessDelta);
        return;
    }

    int midRow = reg.startRow + rows / 2;
    int midCol = reg.startCol + cols / 2;

    Region topLeft     {reg.startRow, midRow,     reg.startCol, midCol};
    Region topRight    {reg.startRow, midRow,     midCol,       reg.endCol};
    Region bottomLeft  {midRow,       reg.endRow, reg.startCol, midCol};
    Region bottomRight {midRow,       reg.endRow, midCol,       reg.endCol};

    if (depth < maxParallelDepth) {
        auto f1 = std::async(std::launch::async, divideAndConquerParallel,
                             std::cref(input), std::ref(output), topLeft,
                             op, brightnessDelta, depth + 1, maxParallelDepth, minBlockSize);
        auto f2 = std::async(std::launch::async, divideAndConquerParallel,
                             std::cref(input), std::ref(output), topRight,
                             op, brightnessDelta, depth + 1, maxParallelDepth, minBlockSize);
        auto f3 = std::async(std::launch::async, divideAndConquerParallel,
                             std::cref(input), std::ref(output), bottomLeft,
                             op, brightnessDelta, depth + 1, maxParallelDepth, minBlockSize);

        divideAndConquerParallel(input, output, bottomRight,
                                 op, brightnessDelta, depth + 1, maxParallelDepth, minBlockSize);
        f1.get();
        f2.get();
        f3.get();
    } else {
        divideAndConquerParallel(input, output, topLeft,     op, brightnessDelta, depth + 1, maxParallelDepth, minBlockSize);
        divideAndConquerParallel(input, output, topRight,    op, brightnessDelta, depth + 1, maxParallelDepth, minBlockSize);
        divideAndConquerParallel(input, output, bottomLeft,  op, brightnessDelta, depth + 1, maxParallelDepth, minBlockSize);
        divideAndConquerParallel(input, output, bottomRight, op, brightnessDelta, depth + 1, maxParallelDepth, minBlockSize);
    }
}

bool verifyImagesMatch(const Image& a, const Image& b) {
    if (a.width != b.width || a.height != b.height) return false;
    for (int r = 0; r < a.height; ++r)
        for (int c = 0; c < a.width; ++c)
            if (!(a.matrix[r][c] == b.matrix[r][c])) return false;
    return true;
}

void runOperation(const Image& input, Operation op, const std::string& name,
                  const std::string& baseName, const fs::path& outDir,
                  int maxParallelDepth, unsigned int leafTasks, unsigned int hwThreads) {
    Region fullImage{0, input.height, 0, input.width};
    int brightnessDelta = 50;

    // 1. Sequential
    Image seqOutput(input.width, input.height);
    auto seqStart = std::chrono::high_resolution_clock::now();
    processRegion(input, seqOutput, fullImage, op, brightnessDelta);
    auto seqEnd = std::chrono::high_resolution_clock::now();
    double seqMs = std::chrono::duration<double, std::milli>(seqEnd - seqStart).count();

    // 2. Parallel Divide & Conquer
    Image parOutput(input.width, input.height);
    auto parStart = std::chrono::high_resolution_clock::now();
    divideAndConquerParallel(input, parOutput, fullImage, op, brightnessDelta,
                             0, maxParallelDepth, 64);
    auto parEnd = std::chrono::high_resolution_clock::now();
    double parMs = std::chrono::duration<double, std::milli>(parEnd - parStart).count();

    bool identical = verifyImagesMatch(seqOutput, parOutput);
    fs::path outPath = outDir / (baseName + "_" + name + ".bmp");
    saveBMP(outPath.string(), parOutput);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n--- [" << name << "] ---\n";
    std::cout << "Sequential time  : " << seqMs << " ms\n";
    std::cout << "Parallel D&C time: " << parMs << " ms (" << leafTasks << " quadrants on " << hwThreads << " cores)\n";
    std::cout << "Speedup          : " << (seqMs / parMs) << "x\n";
    std::cout << "Verification     : " << (identical ? "PASS" : "FAIL") << "\n";
    std::cout << "Saved Output     : " << outPath.string() << "\n";
}

// -------------------- Main --------------------

int main() {
    try {
        fs::path inDir  = "input_images";
        fs::path outDir = "output_images";
        fs::create_directories(inDir);
        fs::create_directories(outDir);

        // Find all .bmp images inside input_images/
        std::vector<fs::path> bmpFiles;
        for (const auto& entry : fs::directory_iterator(inDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".bmp") {
                bmpFiles.push_back(entry.path());
            }
        }

        // If folder is empty, auto-generate a 1600x1200 sample image
        if (bmpFiles.empty()) {
            fs::path samplePath = inDir / "sample.bmp";
            std::cout << "No .bmp files found in '" << inDir.string()
                      << "'. Generating 1600x1200 sample image...\n";
            Image sample = generateSampleImage(1600, 1200);
            saveBMP(samplePath.string(), sample);
            bmpFiles.push_back(samplePath);
        }

        unsigned int hwThreads = std::thread::hardware_concurrency();
        if (hwThreads == 0) hwThreads = 4;

        int maxParallelDepth = 1;
        unsigned int leafTasks = 4;
        while (leafTasks * 4 <= hwThreads * 2) {
            leafTasks *= 4;
            maxParallelDepth++;
        }

        // Process every .bmp image in input_images/
        for (const auto& filePath : bmpFiles) {
            std::cout << "\n=============================================\n";
            std::cout << "Processing: " << filePath.string() << "\n";
            Image input = loadBMP(filePath.string());
            std::cout << "Resolution: " << input.width << "x" << input.height << " pixels\n";

            std::string baseName = filePath.stem().string();
            runOperation(input, Operation::BRIGHTNESS, "brightness", baseName, outDir, maxParallelDepth, leafTasks, hwThreads);
            runOperation(input, Operation::GRAYSCALE,  "grayscale",  baseName, outDir, maxParallelDepth, leafTasks, hwThreads);
            runOperation(input, Operation::BOX_BLUR,   "box_blur",   baseName, outDir, maxParallelDepth, leafTasks, hwThreads);
            runOperation(input, Operation::SHARPEN,    "sharpen",    baseName, outDir, maxParallelDepth, leafTasks, hwThreads);
        }

        std::cout << "\nDone! Open the '" << outDir.string() << "' folder to view all processed images.\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
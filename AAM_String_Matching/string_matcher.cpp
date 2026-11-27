#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <chrono>
#include <set>
#include <limits>
#include <random>

using namespace std;
using namespace std::chrono;

struct MatchResult {
    string name;
    vector<int> positions;
    long long comparisons = 0;
    long long hashComputations = 0;
    long long stateTransitions = 0;
    long long spuriousHits = 0;
    double timeMicros = 0.0;
};

void printAlignment(const string& text, const string& pat, int shift, int compareIdx = -1, bool matched = false) {
    cout << "    Text   : " << text << "\n";
    cout << "    Pattern: " << string(shift, ' ') << pat << "\n";
    if (compareIdx >= 0) {
        cout << "    Check  : " << string(shift + compareIdx, ' ')
             << (matched ? "^ (Match)" : "x (Mismatch)") << "\n";
    }
}

// ============================================================
// 1. NAIVE STRING-MATCHING ALGORITHM
// ============================================================
MatchResult naiveSearch(const string& text, const string& pat, bool verbose) {
    MatchResult res;
    res.name = "Naive";
    int n = text.size();
    int m = pat.size();
    if (m == 0 || m > n) return res;

    auto start = high_resolution_clock::now();

    if (verbose) {
        cout << "\n================ NAIVE ALGORITHM TRACE ================\n";
    }

    for (int s = 0; s <= n - m; ++s) {
        if (verbose) {
            cout << "\n[Iteration " << (s + 1) << "] Shift s = " << s << "\n";
        }
        int j = 0;
        while (j < m) {
            res.comparisons++;
            bool isMatch = (text[s + j] == pat[j]);
            if (verbose) {
                cout << "  Compare text[" << (s + j) << "] ('" << text[s + j]
                     << "') vs pat[" << j << "] ('" << pat[j] << "') -> "
                     << (isMatch ? "MATCH" : "MISMATCH") << "\n";
            }
            if (!isMatch) {
                if (verbose) printAlignment(text, pat, s, j, false);
                break;
            }
            j++;
        }
        if (j == m) {
            res.positions.push_back(s);
            if (verbose) {
                printAlignment(text, pat, s, m - 1, true);
                cout << "  >>> PATTERN FOUND AT INDEX " << s << " <<<\n";
            }
        }
    }

    auto end = high_resolution_clock::now();
    res.timeMicros = duration<double, micro>(end - start).count();
    return res;
}

// ============================================================
// 2. RABIN-KARP ALGORITHM
// ============================================================
MatchResult rabinKarpSearch(const string& text, const string& pat, bool verbose) {
    MatchResult res;
    res.name = "Rabin-Karp";
    int n = text.size();
    int m = pat.size();
    if (m == 0 || m > n) return res;

    const long long d = 256;
    const long long q = 101;

    auto start = high_resolution_clock::now();

    long long h = 1;
    for (int i = 0; i < m - 1; ++i) {
        h = (h * d) % q;
    }

    long long pHash = 0;
    long long tHash = 0;
    for (int i = 0; i < m; ++i) {
        pHash = (d * pHash + (unsigned char)pat[i]) % q;
        tHash = (d * tHash + (unsigned char)text[i]) % q;
    }
    res.hashComputations += 2;

    if (verbose) {
        cout << "\n============= RABIN-KARP ALGORITHM TRACE ==============\n";
        cout << "Base d = " << d << ", Prime q = " << q << ", Pattern Hash = " << pHash << "\n";
    }

    for (int s = 0; s <= n - m; ++s) {
        if (verbose) {
            cout << "\n[Iteration " << (s + 1) << "] Shift s = " << s
                 << " | Window = \"" << text.substr(s, m) << "\""
                 << " | Window Hash = " << tHash << " vs Pattern Hash = " << pHash << "\n";
        }

        if (pHash == tHash) {
            if (verbose) cout << "  Hash matched! Verifying characters...\n";
            bool match = true;
            for (int j = 0; j < m; ++j) {
                res.comparisons++;
                bool charMatch = (text[s + j] == pat[j]);
                if (verbose) {
                    cout << "    Compare text[" << (s + j) << "] ('" << text[s + j]
                         << "') vs pat[" << j << "] ('" << pat[j] << "') -> "
                         << (charMatch ? "MATCH" : "MISMATCH") << "\n";
                }
                if (!charMatch) {
                    match = false;
                    res.spuriousHits++;
                    if (verbose) {
                        printAlignment(text, pat, s, j, false);
                        cout << "  *** SPURIOUS HIT at shift " << s << " ***\n";
                    }
                    break;
                }
            }
            if (match) {
                res.positions.push_back(s);
                if (verbose) {
                    printAlignment(text, pat, s, m - 1, true);
                    cout << "  >>> PATTERN FOUND AT INDEX " << s << " <<<\n";
                }
            }
        } else if (verbose) {
            cout << "  Hash mismatch -> skipping character verification.\n";
        }

        if (s < n - m) {
            tHash = (d * (tHash - (unsigned char)text[s] * h) + (unsigned char)text[s + m]) % q;
            if (tHash < 0) tHash += q;
            res.hashComputations++;
        }
    }

    auto end = high_resolution_clock::now();
    res.timeMicros = duration<double, micro>(end - start).count();
    return res;
}

// ============================================================
// 3. FINITE AUTOMATA ALGORITHM
// ============================================================
MatchResult finiteAutomataSearch(const string& text, const string& pat, bool verbose) {
    MatchResult res;
    res.name = "Finite Automata";
    int n = text.size();
    int m = pat.size();
    if (m == 0 || m > n) return res;

    const int NO_OF_CHARS = 256;
    auto start = high_resolution_clock::now();

    vector<vector<int>> TF(m + 1, vector<int>(NO_OF_CHARS, 0));
    int lps = 0;
    TF[0][(unsigned char)pat[0]] = 1;

    for (int i = 1; i <= m; ++i) {
        for (int x = 0; x < NO_OF_CHARS; ++x) {
            TF[i][x] = TF[lps][x];
        }
        if (i < m) {
            TF[i][(unsigned char)pat[i]] = i + 1;
            lps = TF[lps][(unsigned char)pat[i]];
        }
    }

    if (verbose) {
        cout << "\n=========== FINITE AUTOMATA ALGORITHM TRACE ===========\n";
        set<char> alphabet(pat.begin(), pat.end());
        cout << "Transition Table (State -> Next State):\n";
        cout << setw(8) << "State";
        for (char c : alphabet) cout << setw(6) << c;
        cout << setw(8) << "Other\n";
        cout << string(16 + alphabet.size() * 6, '-') << "\n";
        for (int s = 0; s <= m; ++s) {
            cout << setw(8) << s;
            for (char c : alphabet) cout << setw(6) << TF[s][(unsigned char)c];
            cout << setw(8) << 0 << "\n";
        }
        cout << "\nScanning Text:\n";
    }

    int state = 0;
    for (int i = 0; i < n; ++i) {
        int prevState = state;
        state = TF[state][(unsigned char)text[i]];
        res.stateTransitions++;
        res.comparisons++;

        if (verbose) {
            cout << "[Iteration " << (i + 1) << "] Index i = " << i
                 << " | Char = '" << text[i] << "'"
                 << " | Transition: State " << prevState << " -> " << state << "\n";
        }

        if (state == m) {
            int matchIdx = i - m + 1;
            res.positions.push_back(matchIdx);
            if (verbose) {
                printAlignment(text, pat, matchIdx, m - 1, true);
                cout << "  >>> ACCEPTING STATE REACHED! PATTERN FOUND AT INDEX " << matchIdx << " <<<\n";
            }
        }
    }

    auto end = high_resolution_clock::now();
    res.timeMicros = duration<double, micro>(end - start).count();
    return res;
}

// ============================================================
// 4. KNUTH-MORRIS-PRATT (KMP) ALGORITHM
// ============================================================
MatchResult kmpSearch(const string& text, const string& pat, bool verbose) {
    MatchResult res;
    res.name = "KMP";
    int n = text.size();
    int m = pat.size();
    if (m == 0 || m > n) return res;

    auto start = high_resolution_clock::now();

    vector<int> lps(m, 0);
    int len = 0;
    int idx = 1;
    while (idx < m) {
        if (pat[idx] == pat[len]) {
            len++;
            lps[idx] = len;
            idx++;
        } else {
            if (len != 0) {
                len = lps[len - 1];
            } else {
                lps[idx] = 0;
                idx++;
            }
        }
    }

    if (verbose) {
        cout << "\n================= KMP ALGORITHM TRACE =================\n";
        cout << "LPS Table:\n  Index : ";
        for (int k = 0; k < m; ++k) cout << setw(4) << k;
        cout << "\n  Char  : ";
        for (int k = 0; k < m; ++k) cout << setw(4) << pat[k];
        cout << "\n  LPS   : ";
        for (int k = 0; k < m; ++k) cout << setw(4) << lps[k];
        cout << "\n\nSearching Text:\n";
    }

    int i = 0;
    int j = 0;
    int iter = 0;

    while (i < n) {
        iter++;
        res.comparisons++;
        bool isMatch = (text[i] == pat[j]);

        if (verbose) {
            cout << "\n[Iteration " << iter << "] Compare text[" << i << "] ('" << text[i]
                 << "') vs pat[" << j << "] ('" << pat[j] << "') -> "
                 << (isMatch ? "MATCH" : "MISMATCH") << "\n";
            printAlignment(text, pat, i - j, j, isMatch);
        }

        if (isMatch) {
            i++;
            j++;
            if (j == m) {
                int matchIdx = i - j;
                res.positions.push_back(matchIdx);
                if (verbose) {
                    cout << "  >>> PATTERN FOUND AT INDEX " << matchIdx << " <<<\n";
                    cout << "  Reset j using lps[" << (j - 1) << "] = " << lps[j - 1] << "\n";
                }
                j = lps[j - 1];
            }
        } else {
            if (j != 0) {
                if (verbose) {
                    cout << "  Mismatch -> Shift pattern using lps[" << (j - 1)
                         << "] = " << lps[j - 1] << " (no backtracking on text index i)\n";
                }
                j = lps[j - 1];
            } else {
                if (verbose) {
                    cout << "  Mismatch at j = 0 -> Advance text index i to " << (i + 1) << "\n";
                }
                i++;
            }
        }
    }

    auto end = high_resolution_clock::now();
    res.timeMicros = duration<double, micro>(end - start).count();
    return res;
}

// ============================================================
// REPORTING & BENCHMARKING UTILITIES
// ============================================================
void printSummaryTable(const vector<MatchResult>& results) {
    cout << "\n=========================================================================================\n";
    cout << "                              ALGORITHM PERFORMANCE SUMMARY                              \n";
    cout << "=========================================================================================\n";
    cout << left << setw(17) << "Algorithm"
         << setw(14) << "Matches"
         << setw(15) << "Comparisons"
         << setw(12) << "Hashes"
         << setw(15) << "Transitions"
         << setw(14) << "Time (us)" << "\n";
    cout << "-----------------------------------------------------------------------------------------\n";

    for (const auto& r : results) {
        cout << left << setw(17) << r.name
             << setw(14) << r.positions.size()
             << setw(15) << r.comparisons
             << setw(12) << r.hashComputations
             << setw(15) << r.stateTransitions
             << fixed << setprecision(2) << setw(14) << r.timeMicros << "\n";
    }
    cout << "-----------------------------------------------------------------------------------------\n";

    cout << "Matching Positions:\n";
    for (const auto& r : results) {
        cout << "  " << left << setw(16) << r.name << ": ";
        if (r.positions.empty()) {
            cout << "None";
        } else {
            for (size_t k = 0; k < r.positions.size(); ++k) {
                cout << r.positions[k] << (k + 1 < r.positions.size() ? ", " : "");
            }
        }
        if (r.name == "Rabin-Karp") {
            cout << "  (Spurious Hits: " << r.spuriousHits << ")";
        }
        cout << "\n";
    }
    cout << "=========================================================================================\n";
}

void runBenchmark() {
    vector<int> sizes = {1000, 10000, 50000, 100000};
    string pat = "ABABACABAB";
    mt19937 rng(42);
    string alphabet = "ABC";
    uniform_int_distribution<int> dist(0, alphabet.size() - 1);

    cout << "\n=================================================================================\n";
    cout << "        EXECUTION TIME BENCHMARK (Pattern: \"" << pat << "\", Alphabet: {A,B,C})\n";
    cout << "=================================================================================\n";
    cout << left << setw(12) << "Text Size"
         << setw(16) << "Naive (us)"
         << setw(18) << "Rabin-Karp (us)"
         << setw(20) << "Finite Auto. (us)"
         << setw(14) << "KMP (us)" << "\n";
    cout << "---------------------------------------------------------------------------------\n";

    for (int sz : sizes) {
        string text(sz, 'A');
        for (int i = 0; i < sz; ++i) text[i] = alphabet[dist(rng)];

        auto r1 = naiveSearch(text, pat, false);
        auto r2 = rabinKarpSearch(text, pat, false);
        auto r3 = finiteAutomataSearch(text, pat, false);
        auto r4 = kmpSearch(text, pat, false);

        cout << left << setw(12) << sz
             << fixed << setprecision(2)
             << setw(16) << r1.timeMicros
             << setw(18) << r2.timeMicros
             << setw(20) << r3.timeMicros
             << setw(14) << r4.timeMicros << "\n";
    }
    cout << "=================================================================================\n";
}

int main() {
    string text = "ABABDABACDABABCABAB";
    string pat = "ABABCABAB";

    cout << "====================================================\n";
    cout << "    STRING MATCHING ALGORITHM COMPARISON PLATFORM   \n";
    cout << "====================================================\n";

    while (true) {
        cout << "\nCurrent Text   : \"" << text << "\" (len=" << text.size() << ")\n";
        cout << "Current Pattern: \"" << pat << "\" (len=" << pat.size() << ")\n";
        cout << "\nMenu:\n";
        cout << "  1. Enter New Text and Pattern\n";
        cout << "  2. Run Naive String-Matching (with Trace)\n";
        cout << "  3. Run Rabin-Karp (with Trace)\n";
        cout << "  4. Run Finite Automata (with Trace)\n";
        cout << "  5. Run KMP (with Trace)\n";
        cout << "  6. Run & Compare ALL 4 Algorithms (with Trace)\n";
        cout << "  7. Benchmark Execution Times Across Input Sizes\n";
        cout << "  0. Exit\n";
        cout << "Select an option: ";

        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 0) {
            cout << "Exiting platform. Goodbye!\n";
            break;
        }

        switch (choice) {
            case 1:
                cout << "Enter Text: ";
                getline(cin, text);
                cout << "Enter Pattern: ";
                getline(cin, pat);
                break;
            case 2:
                printSummaryTable({naiveSearch(text, pat, true)});
                break;
            case 3:
                printSummaryTable({rabinKarpSearch(text, pat, true)});
                break;
            case 4:
                printSummaryTable({finiteAutomataSearch(text, pat, true)});
                break;
            case 5:
                printSummaryTable({kmpSearch(text, pat, true)});
                break;
            case 6: {
                vector<MatchResult> all;
                all.push_back(naiveSearch(text, pat, true));
                all.push_back(rabinKarpSearch(text, pat, true));
                all.push_back(finiteAutomataSearch(text, pat, true));
                all.push_back(kmpSearch(text, pat, true));
                printSummaryTable(all);
                break;
            }
            case 7:
                runBenchmark();
                break;
            default:
                cout << "Invalid choice. Try again.\n";
        }
    }

    return 0;
}
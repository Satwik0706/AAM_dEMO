#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <string>
#include <stdexcept>

using namespace std;

const double EPS = 1e-9;
const double BIG_M = 1e6;

class SimplexSolver {
private:
    int n, m;
    bool maximize;
    vector<vector<double>> A;
    vector<double> b, c;
    vector<string> signs;
    vector<string> varNames;
    vector<int> basis;
    vector<vector<double>> T;
    int iterations = 0;

    void printTableau(string title) {
        cout << "\n" << title << "\n";

        cout << setw(10) << "Basis";

        for (int j = 0; j < (int)varNames.size(); j++)
            cout << setw(10) << varNames[j];

        cout << setw(12) << "RHS" << "\n";

        for (int i = 0; i < m; i++) {
            cout << setw(10) << varNames[basis[i]];

            for (int j = 0; j < (int)varNames.size(); j++)
                cout << setw(10) << fixed << setprecision(2)
                     << T[i][j];

            cout << setw(12) << T[i].back() << "\n";
        }

        cout << setw(10) << "Zj-Cj";

        for (int j = 0; j < (int)varNames.size(); j++)
            cout << setw(10) << T[m][j];

        cout << setw(12) << T[m].back() << "\n";
    }

    void pivot(int row, int col) {
        double p = T[row][col];

        for (int j = 0; j <= (int)varNames.size(); j++)
            T[row][j] /= p;

        for (int i = 0; i <= m; i++) {
            if (i == row) continue;

            double factor = T[i][col];

            for (int j = 0; j <= (int)varNames.size(); j++)
                T[i][j] -= factor * T[row][j];
        }

        basis[row] = col;
    }

    void calculateZjCj() {
        int total = varNames.size();

        for (int j = 0; j < total; j++) {
            T[m][j] = 0;

            for (int i = 0; i < m; i++)
                T[m][j] += c[basis[i]] * T[i][j];

            T[m][j] -= c[j];
        }

        T[m].back() = 0;

        for (int i = 0; i < m; i++)
            T[m].back() += c[basis[i]] * T[i].back();
    }

public:
    void input() {
        cout << "====================================\n";
        cout << " LINEAR PROGRAMMING PROBLEM SOLVER\n";
        cout << "====================================\n";

        cout << "\nEnter number of variables: ";
        cin >> n;

        cout << "Enter number of constraints: ";
        cin >> m;

        if (n <= 0 || m <= 0) {
            throw runtime_error("Variables and constraints must be positive.");
        }

        int choice;

        cout << "\n1. Maximization\n2. Minimization\n";
        cout << "Enter choice: ";
        cin >> choice;

        if (choice != 1 && choice != 2)
            throw runtime_error("Invalid optimization choice.");

        maximize = (choice == 1);

        A.assign(m, vector<double>(n));
        b.resize(m);
        c.resize(n);
        signs.resize(m);

        cout << "\nEnter objective function coefficients:\n";

        for (int j = 0; j < n; j++) {
            cout << "Coefficient of x" << j + 1 << ": ";
            cin >> c[j];

            if (!maximize)
                c[j] = -c[j];
        }

        cout << "\nEnter constraints:\n";

        for (int i = 0; i < m; i++) {
            cout << "\nConstraint " << i + 1 << ":\n";

            for (int j = 0; j < n; j++) {
                cout << "Coefficient of x" << j + 1 << ": ";
                cin >> A[i][j];
            }

            cout << "Enter relation (<=, >=, =): ";
            cin >> signs[i];

            if (signs[i] != "<=" &&
                signs[i] != ">=" &&
                signs[i] != "=")
                throw runtime_error("Invalid constraint relation.");

            cout << "Enter RHS value: ";
            cin >> b[i];

            if (!cin)
                throw runtime_error("Invalid numeric input.");

            // Normalize negative RHS.
            if (b[i] < 0) {
                b[i] = -b[i];

                for (int j = 0; j < n; j++)
                    A[i][j] = -A[i][j];

                if (signs[i] == "<=")
                    signs[i] = ">=";
                else if (signs[i] == ">=")
                    signs[i] = "<=";
            }
        }
    }

    void convertToStandardForm() {
        varNames.clear();

        for (int j = 0; j < n; j++)
            varNames.push_back("x" + to_string(j + 1));

        vector<vector<double>> newA(m);
        basis.resize(m);

        // First count additional variables.
        int total = n;

        for (int i = 0; i < m; i++) {
            if (signs[i] == "<=") total++;
            else if (signs[i] == ">=") total += 2;
            else total++;
        }

        c.resize(total, 0);

        for (int i = 0; i < m; i++) {
            newA[i].assign(total, 0);

            for (int j = 0; j < n; j++)
                newA[i][j] = A[i][j];
        }

        int col = n;

        for (int i = 0; i < m; i++) {
            if (signs[i] == "<=") {
                newA[i][col] = 1;
                basis[i] = col;
                varNames.push_back("S" + to_string(i + 1));
                col++;
            }
            else if (signs[i] == ">=") {
                newA[i][col] = -1;
                varNames.push_back("S" + to_string(i + 1));
                col++;

                newA[i][col] = 1;
                basis[i] = col;
                varNames.push_back("A" + to_string(i + 1));

                c[col] = -BIG_M;
                col++;
            }
            else {
                newA[i][col] = 1;
                basis[i] = col;
                varNames.push_back("A" + to_string(i + 1));

                c[col] = -BIG_M;
                col++;
            }
        }

        A = newA;

        // Create tableau.
        T.assign(m + 1, vector<double>(total + 1, 0));

        for (int i = 0; i < m; i++) {
            for (int j = 0; j < total; j++)
                T[i][j] = A[i][j];

            T[i][total] = b[i];
        }

        cout << "\nSTANDARD FORM:\n";

        for (int i = 0; i < m; i++) {
            cout << "Constraint " << i + 1 << ": ";

            for (int j = 0; j < total; j++) {
                if (abs(A[i][j]) > EPS)
                    cout << A[i][j] << varNames[j] << " ";
            }

            cout << "= " << b[i] << "\n";
        }

        calculateZjCj();
        printTableau("INITIAL SIMPLEX TABLEAU");
    }

    void solve() {
        while (true) {
            int total = varNames.size();

            int entering = -1;
            double mostNegative = -EPS;

            // Find entering variable.
            for (int j = 0; j < total; j++) {
                if (T[m][j] < mostNegative) {
                    mostNegative = T[m][j];
                    entering = j;
                }
            }

            // Optimal solution found.
            if (entering == -1)
                break;

            int leaving = -1;
            double minRatio = 1e100;

            // Find leaving variable using minimum ratio test.
            for (int i = 0; i < m; i++) {
                if (T[i][entering] > EPS) {
                    double ratio = T[i].back() / T[i][entering];

                    if (ratio < minRatio) {
                        minRatio = ratio;
                        leaving = i;
                    }
                }
            }

            // Unbounded case.
            if (leaving == -1) {
                cout << "\nRESULT: UNBOUNDED SOLUTION!\n";
                return;
            }

            iterations++;

            cout << "\nEntering variable: "
                 << varNames[entering];

            cout << "\nLeaving variable: "
                 << varNames[basis[leaving]];

            pivot(leaving, entering);

            calculateZjCj();

            printTableau("ITERATION " + to_string(iterations));
        }

        // Check for artificial variables in the final basis.
        for (int i = 0; i < m; i++) {
            if (varNames[basis[i]][0] == 'A' &&
                T[i].back() > EPS) {
                cout << "\nRESULT: INFEASIBLE PROBLEM!\n";
                return;
            }
        }

        cout << "\n====================================\n";
        cout << "       OPTIMAL SOLUTION FOUND\n";
        cout << "====================================\n";

        vector<double> solution(n, 0);

        for (int i = 0; i < m; i++) {
            if (basis[i] < n)
                solution[basis[i]] = T[i].back();
        }

        for (int j = 0; j < n; j++) {
            cout << "x" << j + 1 << " = "
                 << fixed << setprecision(4)
                 << solution[j] << "\n";
        }

        double answer = T[m].back();

        if (!maximize)
            answer = -answer;

        cout << "\nOptimal objective value = "
             << answer << "\n";
    }

    void run() {
        input();

        auto start = chrono::high_resolution_clock::now();

        convertToStandardForm();
        solve();

        auto stop = chrono::high_resolution_clock::now();

        double elapsed =
            chrono::duration<double, milli>(stop - start).count();

        cout << "\n====================================\n";
        cout << "       PERFORMANCE ANALYSIS\n";
        cout << "====================================\n";

        cout << "Total iterations: " << iterations << "\n";
        cout << "Computation time: "
             << fixed << setprecision(4)
             << elapsed << " milliseconds\n";
    }
};

int main() {
    try {
        SimplexSolver solver;
        solver.run();
    }
    catch (const exception& e) {
        cout << "\nError: " << e.what() << "\n";
    }

    return 0;
}
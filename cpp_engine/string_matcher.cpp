/**
 * ============================================================================
 * ADVANCED ALGORITHMS (23CS5PEAAM) - AAT PROJECT
 * Problem 3: String Matching Algorithm Comparison Platform (C++ Engine)
 * 
 * Algorithms Implemented:
 *   1. Naive String Matching (Brute-Force)
 *   2. Rabin-Karp Algorithm (Polynomial Rolling Hash)
 *   3. Finite Automata Matcher (DFA Transition State Machine)
 *   4. Knuth-Morris-Pratt (KMP Prefix Function / LPS Array)
 * 
 * Features:
 *   - Character comparisons tracking
 *   - Hash computations & collision / spurious hit detection
 *   - Step-by-step visual terminal tracing
 *   - High-precision execution benchmarking (microseconds)
 *   - Comparative summary matrix
 * ============================================================================
 */

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <map>
#include <set>
#include <algorithm>

using namespace std;

// Structure to store execution results of an algorithm
struct MatchResult {
    string algorithmName;
    vector<int> matches;
    long long comparisons;
    long long extraOperations; // e.g. hash computations or state transitions
    double executionTimeUs;
    string timeComplexity;
    string spaceComplexity;
};

// ----------------------------------------------------------------------------
// 1. NAIVE STRING MATCHING ALGORITHM
// ----------------------------------------------------------------------------
MatchResult runNaive(const string& text, const string& pattern, bool verbose = false) {
    auto start = chrono::high_resolution_clock::now();
    MatchResult res;
    res.algorithmName = "Naive String Matching";
    res.comparisons = 0;
    res.extraOperations = 0;
    res.timeComplexity = "O((n - m + 1) * m)";
    res.spaceComplexity = "O(1)";

    int n = (int)text.length();
    int m = (int)pattern.length();

    if (m == 0 || n == 0 || m > n) {
        auto end = chrono::high_resolution_clock::now();
        res.executionTimeUs = chrono::duration<double, micro>(end - start).count();
        return res;
    }

    if (verbose) {
        cout << "\n--- [Naive Matching Step-by-Step Trace] ---\n";
    }

    for (int s = 0; s <= n - m; ++s) {
        bool match = true;
        for (int j = 0; j < m; ++j) {
            res.comparisons++;
            if (text[s + j] != pattern[j]) {
                match = false;
                if (verbose) {
                    cout << " Shift " << setw(2) << s << ": Mismatch at text[" << (s + j) 
                         << "]='" << text[s + j] << "' != pattern[" << j << "]='" << pattern[j] << "'\n";
                }
                break;
            }
        }
        if (match) {
            res.matches.push_back(s);
            if (verbose) {
                cout << " >>> COMPLETE MATCH at shift " << s << " (1-based: " << (s + 1) << ")!\n";
            }
        }
    }

    auto end = chrono::high_resolution_clock::now();
    res.executionTimeUs = chrono::duration<double, micro>(end - start).count();
    return res;
}

// ----------------------------------------------------------------------------
// 2. RABIN-KARP ALGORITHM (Rolling Hash)
// ----------------------------------------------------------------------------
MatchResult runRabinKarp(const string& text, const string& pattern, int d = 256, int q = 101, bool verbose = false) {
    auto start = chrono::high_resolution_clock::now();
    MatchResult res;
    res.algorithmName = "Rabin-Karp Algorithm";
    res.comparisons = 0;
    res.extraOperations = 0; // Tracks hash operations + collisions
    res.timeComplexity = "Average O(n + m), Worst O(n * m)";
    res.spaceComplexity = "O(1)";

    int n = (int)text.length();
    int m = (int)pattern.length();

    if (m == 0 || n == 0 || m > n) {
        auto end = chrono::high_resolution_clock::now();
        res.executionTimeUs = chrono::duration<double, micro>(end - start).count();
        return res;
    }

    // h = (d^(m-1)) % q
    int h = 1;
    for (int i = 0; i < m - 1; ++i) {
        h = (h * d) % q;
    }

    int p = 0; // Pattern hash
    int t = 0; // Current window hash
    for (int i = 0; i < m; ++i) {
        p = (d * p + (unsigned char)pattern[i]) % q;
        t = (d * t + (unsigned char)text[i]) % q;
        res.extraOperations += 2;
    }

    if (verbose) {
        cout << "\n--- [Rabin-Karp Rolling Hash Trace (Modulus q=" << q << ", Base d=" << d << ")] ---\n";
        cout << " Pattern Hash (p) = " << p << " | Initial Window Hash (t0) = " << t << "\n";
    }

    int collisions = 0;
    for (int s = 0; s <= n - m; ++s) {
        if (p == t) {
            // Hashes match, verify characters
            bool match = true;
            for (int j = 0; j < m; ++j) {
                res.comparisons++;
                if (text[s + j] != pattern[j]) {
                    match = false;
                    collisions++;
                    if (verbose) {
                        cout << " Shift " << setw(2) << s << ": SPURIOUS HIT / COLLISION! Hash matched (" 
                             << t << ") but characters differ.\n";
                    }
                    break;
                }
            }
            if (match) {
                res.matches.push_back(s);
                if (verbose) {
                    cout << " >>> COMPLETE MATCH at shift " << s << " (Hash " << t << " verified)!\n";
                }
            }
        } else {
            if (verbose) {
                cout << " Shift " << setw(2) << s << ": Hashes differ (t=" << t << " != p=" << p << "). Skipped in O(1).\n";
            }
        }

        // Rolling hash update for next window
        if (s < n - m) {
            t = (d * (t - (unsigned char)text[s] * h) + (unsigned char)text[s + m]) % q;
            if (t < 0) t += q;
            res.extraOperations++;
        }
    }

    auto end = chrono::high_resolution_clock::now();
    res.executionTimeUs = chrono::duration<double, micro>(end - start).count();
    return res;
}

// ----------------------------------------------------------------------------
// 3. FINITE AUTOMATA MATCHER
// ----------------------------------------------------------------------------
MatchResult runFiniteAutomata(const string& text, const string& pattern, bool verbose = false) {
    auto start = chrono::high_resolution_clock::now();
    MatchResult res;
    res.algorithmName = "Finite Automata Matcher";
    res.comparisons = 0;
    res.extraOperations = 0; // State transitions
    res.timeComplexity = "Preprocessing O(m*|Sigma|), Matching O(n)";
    res.spaceComplexity = "O(m * |Sigma|)";

    int n = (int)text.length();
    int m = (int)pattern.length();

    if (m == 0 || n == 0 || m > n) {
        auto end = chrono::high_resolution_clock::now();
        res.executionTimeUs = chrono::duration<double, micro>(end - start).count();
        return res;
    }

    // Build unique alphabet Sigma
    set<char> alphabetSet;
    for (char c : text) alphabetSet.insert(c);
    for (char c : pattern) alphabetSet.insert(c);
    vector<char> alphabet(alphabetSet.begin(), alphabetSet.end());

    // Build transition table: delta[state][char]
    // delta(q, a) = max { k <= m | P_k is suffix of P_q + a }
    vector<map<char, int>> delta(m + 1);
    for (int q = 0; q <= m; ++q) {
        for (char a : alphabet) {
            int k = min(m, q + 1);
            string pq_a = pattern.substr(0, q) + a;
            while (k > 0) {
                string pk = pattern.substr(0, k);
                if (pq_a.length() >= (size_t)k && pq_a.substr(pq_a.length() - k) == pk) {
                    break;
                }
                k--;
            }
            delta[q][a] = k;
        }
    }

    if (verbose) {
        cout << "\n--- [Finite Automata DFA Transition Matrix (|Sigma|=" << alphabet.size() << ")] ---\n";
        cout << " State | ";
        for (char c : alphabet) cout << "'" << c << "' ";
        cout << "\n-------+-" << string(alphabet.size() * 4, '-') << "\n";
        for (int q = 0; q <= m; ++q) {
            cout << "   " << setw(2) << q << "  | ";
            for (char c : alphabet) {
                cout << " " << setw(2) << delta[q][c] << " ";
            }
            if (q == m) cout << " (ACCEPTING)";
            cout << "\n";
        }
    }

    // DFA Matching Phase: strictly O(n)
    int q = 0;
    for (int i = 0; i < n; ++i) {
        char c = text[i];
        int prev_q = q;
        q = delta[prev_q][c];
        res.extraOperations++; // State transition

        if (verbose) {
            cout << " Text[" << setw(2) << i << "]='" << c << "': State " << prev_q 
                 << " -> State " << q << (q == m ? " [ACCEPT!]" : "") << "\n";
        }

        if (q == m) {
            int shift = i - m + 1;
            res.matches.push_back(shift);
        }
    }

    auto end = chrono::high_resolution_clock::now();
    res.executionTimeUs = chrono::duration<double, micro>(end - start).count();
    return res;
}

// ----------------------------------------------------------------------------
// 4. KNUTH-MORRIS-PRATT (KMP) ALGORITHM
// ----------------------------------------------------------------------------
vector<int> computePi(const string& pattern) {
    int m = (int)pattern.length();
    vector<int> pi(m, 0);
    int k = 0;
    for (int q = 1; q < m; ++q) {
        while (k > 0 && pattern[k] != pattern[q]) {
            k = pi[k - 1];
        }
        if (pattern[k] == pattern[q]) {
            k++;
        }
        pi[q] = k;
    }
    return pi;
}

MatchResult runKMP(const string& text, const string& pattern, bool verbose = false) {
    auto start = chrono::high_resolution_clock::now();
    MatchResult res;
    res.algorithmName = "Knuth-Morris-Pratt (KMP)";
    res.comparisons = 0;
    res.extraOperations = 0; // Smart shifts using pi
    res.timeComplexity = "Preprocessing O(m), Matching O(n)";
    res.spaceComplexity = "O(m)";

    int n = (int)text.length();
    int m = (int)pattern.length();

    if (m == 0 || n == 0 || m > n) {
        auto end = chrono::high_resolution_clock::now();
        res.executionTimeUs = chrono::duration<double, micro>(end - start).count();
        return res;
    }

    vector<int> pi = computePi(pattern);

    if (verbose) {
        cout << "\n--- [KMP Prefix Function (pi / LPS Array)] ---\n";
        cout << " Index i : ";
        for (int i = 0; i < m; ++i) cout << setw(3) << i << " ";
        cout << "\n P[i]    : ";
        for (int i = 0; i < m; ++i) cout << setw(3) << pattern[i] << " ";
        cout << "\n pi[i]   : ";
        for (int i = 0; i < m; ++i) cout << setw(3) << pi[i] << " ";
        cout << "\n----------------------------------------------\n";
    }

    int q = 0; // Characters matched
    for (int i = 0; i < n; ++i) {
        while (q > 0 && pattern[q] != text[i]) {
            res.comparisons++;
            if (verbose) {
                cout << " Fallback: text[" << i << "]='" << text[i] << "' != pattern[" << q 
                     << "]='" << pattern[q] << "' -> pi[" << (q - 1) << "] = " << pi[q - 1] << "\n";
            }
            q = pi[q - 1];
            res.extraOperations++;
        }

        res.comparisons++;
        if (pattern[q] == text[i]) {
            q++;
        }

        if (q == m) {
            int shift = i - m + 1;
            res.matches.push_back(shift);
            if (verbose) {
                cout << " >>> COMPLETE MATCH at shift " << shift << " (indices " << shift << ".." << i << ")!\n";
            }
            q = pi[q - 1];
            res.extraOperations++;
        }
    }

    auto end = chrono::high_resolution_clock::now();
    res.executionTimeUs = chrono::duration<double, micro>(end - start).count();
    return res;
}

// ----------------------------------------------------------------------------
// DISPLAY HELPER
// ----------------------------------------------------------------------------
void printComparisonTable(const vector<MatchResult>& results) {
    cout << "\n=======================================================================================================\n";
    cout << "                         ALL 4 ALGORITHMS COMPARATIVE PERFORMANCE BENCHMARK                            \n";
    cout << "=======================================================================================================\n";
    cout << "| " << left << setw(26) << "Algorithm Name" 
         << " | " << right << setw(7) << "Matches"
         << " | " << setw(12) << "Comparisons"
         << " | " << setw(14) << "Time (microsec)"
         << " | " << left << setw(18) << "Time Complexity"
         << " | " << setw(10) << "Space" << " |\n";
    cout << "+----------------------------+---------+--------------+----------------+--------------------+------------+\n";

    for (const auto& r : results) {
        cout << "| " << left << setw(26) << r.algorithmName 
             << " | " << right << setw(7) << r.matches.size()
             << " | " << setw(12) << r.comparisons
             << " | " << setw(14) << fixed << setprecision(2) << r.executionTimeUs
             << " | " << left << setw(18) << r.timeComplexity
             << " | " << setw(10) << r.spaceComplexity << " |\n";
    }
    cout << "=======================================================================================================\n";
}

int main(int argc, char* argv[]) {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    cout << "======================================================================\n";
    cout << "   STRING MATCHING ALGORITHM COMPARISON PLATFORM - C++ SIMULATOR      \n";
    cout << "          Naive | Rabin-Karp | Finite Automata | KMP                  \n";
    cout << "======================================================================\n";

    string text = "AABAACAADAABAABA";
    string pattern = "AABA";

    if (argc >= 3) {
        text = argv[1];
        pattern = argv[2];
    }

    cout << "\nTarget Text (Length " << text.length() << ") : \"" << text << "\"\n";
    cout << "Pattern     (Length " << pattern.length() << ") : \"" << pattern << "\"\n";

    cout << "\nRunning detailed execution trace for all 4 algorithms...\n";

    MatchResult r1 = runNaive(text, pattern, true);
    MatchResult r2 = runRabinKarp(text, pattern, 256, 101, true);
    MatchResult r3 = runFiniteAutomata(text, pattern, true);
    MatchResult r4 = runKMP(text, pattern, true);

    vector<MatchResult> all = {r1, r2, r3, r4};
    printComparisonTable(all);

    cout << "\nMatching Indices Summary:\n";
    cout << " Found at 0-based shifts: { ";
    for (int idx : r4.matches) cout << idx << " ";
    cout << "}\n";
    cout << " Found at 1-based positions: { ";
    for (int idx : r4.matches) cout << (idx + 1) << " ";
    cout << "}\n";

    return 0;
}

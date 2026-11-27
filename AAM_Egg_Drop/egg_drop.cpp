#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
using namespace std;

// Structure to store DP results
struct EggDropResult {
    int eggs, floors, attempts;
    vector<vector<int>> dp;
    vector<vector<int>> decision;
};

// Function to calculate minimum attempts
EggDropResult solveEggDrop(int eggs, int floors) {

    EggDropResult result;

    result.eggs = eggs;
    result.floors = floors;

    vector<vector<int>> dp(
        eggs + 1, vector<int>(floors + 1, 0)
    );

    vector<vector<int>> decision(
        eggs + 1, vector<int>(floors + 1, 0)
    );

    // Base cases
    for (int e = 1; e <= eggs; e++) {
        dp[e][0] = 0;

        if (floors >= 1) {
            dp[e][1] = 1;
            decision[e][1] = 1;
        }
    }

    for (int f = 1; f <= floors; f++) {
        dp[1][f] = f;
        decision[1][f] = 1;
    }

    // Fill DP table
    for (int e = 2; e <= eggs; e++) {

        for (int f = 2; f <= floors; f++) {

            dp[e][f] = f + 1;

            for (int x = 1; x <= f; x++) {

                int breakCase = dp[e - 1][x - 1];

                int surviveCase = dp[e][f - x];

                int attempts = 1 +
                    max(breakCase, surviveCase);

                if (attempts < dp[e][f]) {

                    dp[e][f] = attempts;

                    decision[e][f] = x;
                }
            }
        }
    }

    result.attempts = dp[eggs][floors];
    result.dp = dp;
    result.decision = decision;

    return result;
}

// Display DP Table
void displayDPTable(const EggDropResult &r) {

    cout << "\n========== DP TABLE ==========\n\n";

    cout << setw(8) << "Eggs/F";

    for (int f = 0; f <= r.floors; f++) {
        cout << setw(5) << f;
    }

    cout << "\n";

    for (int e = 1; e <= r.eggs; e++) {

        cout << setw(8) << e;

        for (int f = 0; f <= r.floors; f++) {

            cout << setw(5) << r.dp[e][f];
        }

        cout << "\n";
    }
}

// Display decisions
void displayDecisions(const EggDropResult &r) {

    cout << "\n======= OPTIMAL FLOOR DECISIONS =======\n";

    cout << "Eggs\tFloors\tTest Floor\tAttempts\n";

    for (int e = 1; e <= r.eggs; e++) {

        for (int f = 1; f <= r.floors; f++) {

            cout << e << "\t"
                 << f << "\t"
                 << r.decision[e][f] << "\t\t"
                 << r.dp[e][f] << "\n";
        }
    }
}

// Reconstruct optimal strategy
void reconstructStrategy(
    const EggDropResult &r,
    int eggs,
    int floors,
    int offset = 0
) {

    if (floors <= 0 || eggs <= 0) {
        return;
    }

    if (eggs == 1) {

        cout << "Test floors sequentially: ";

        for (int i = 1; i <= floors; i++) {
            cout << offset + i << " ";
        }

        cout << "\n";
        return;
    }

    int x = r.decision[eggs][floors];

    int testFloor = offset + x;

    cout << "\nDrop egg from floor "
         << testFloor << "\n";

    cout << "If egg BREAKS:\n";

    reconstructStrategy(
        r, eggs - 1, x - 1, offset
    );

    cout << "If egg DOES NOT BREAK:\n";

    reconstructStrategy(
        r, eggs, floors - x, testFloor
    );
}

// Compare different combinations
void compareResults() {

    int n;

    cout << "\nEnter number of test cases: ";
    cin >> n;

    if (n <= 0) {
        cout << "Invalid number of test cases!\n";
        return;
    }

    cout << "\nComparison Results\n";
    cout << "Eggs\tFloors\tMinimum Attempts\n";

    for (int i = 0; i < n; i++) {

        int eggs, floors;

        cout << "\nEnter eggs and floors: ";
        cin >> eggs >> floors;

        if (eggs <= 0 || floors < 0) {
            cout << "Invalid input! Try again.\n";
            i--;
            continue;
        }

        EggDropResult r = solveEggDrop(eggs, floors);

        cout << eggs << "\t"
             << floors << "\t"
             << r.attempts << "\n";
    }
}

// Main function
int main() {

    int choice;

    do {

        cout << "\n====================================\n";
        cout << "   OPTIMAL TESTING STRATEGY VISUALIZER\n";
        cout << "====================================\n";

        cout << "1. Find Minimum Attempts\n";
        cout << "2. Display DP Table\n";
        cout << "3. Display Floor Decisions\n";
        cout << "4. Reconstruct Optimal Strategy\n";
        cout << "5. Compare Different Combinations\n";
        cout << "6. Exit\n";

        cout << "\nEnter your choice: ";
        cin >> choice;

        if (choice >= 1 && choice <= 4) {

            int eggs, floors;

            cout << "\nEnter number of eggs: ";
            cin >> eggs;

            cout << "Enter number of floors: ";
            cin >> floors;

            if (eggs <= 0 || floors < 0) {
                cout << "Invalid input! Eggs must be "
                     << "positive and floors non-negative.\n";
                continue;
            }

            EggDropResult r = solveEggDrop(eggs, floors);

            switch (choice) {

                case 1:
                    cout << "\nMinimum attempts required: "
                         << r.attempts << "\n";
                    break;

                case 2:
                    displayDPTable(r);
                    cout << "\nMinimum attempts: "
                         << r.attempts << "\n";
                    break;

                case 3:
                    displayDecisions(r);
                    break;

                case 4:
                    cout << "\nMinimum attempts required: "
                         << r.attempts << "\n";

                    cout << "\nOptimal Testing Strategy:\n";

                    reconstructStrategy(
                        r, eggs, floors
                    );
                    break;
            }
        }

        else if (choice == 5) {
            compareResults();
        }

        else if (choice == 6) {
            cout << "\nProgram terminated successfully!\n";
        }

        else {
            cout << "\nInvalid choice! Try again.\n";
        }

    } while (choice != 6);

    return 0;
}
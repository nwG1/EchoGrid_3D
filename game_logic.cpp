#include "game_logic.h"
#include <cstdlib> // For rand()

// --- Function Definitions ---
// This is the actual implementation of the game rules.

bool checkWin(const std::vector<char>& board, char playerSymbol) {
    const int wins[8][3] = { {0,1,2}, {3,4,5}, {6,7,8}, {0,3,6}, {1,4,7}, {2,5,8}, {0,4,8}, {2,4,6} };
    for (int i = 0; i < 8; ++i) {
        if (board[wins[i][0]] == playerSymbol && board[wins[i][1]] == playerSymbol && board[wins[i][2]] == playerSymbol) {
            return true;
        }
    }
    return false;
}

bool checkDraw(const std::vector<char>& board) {
    for (char c : board) {
        if (c == ' ') return false;
    }
    return true;
}

int findBestMove(std::vector<char> board) {
    // 1. Check for a winning move for the AI
    for (int i = 0; i < 9; ++i) {
        if (board[i] == ' ') {
            std::vector<char> tempBoard = board;
            tempBoard[i] = P2_SYMBOL;
            if (checkWin(tempBoard, P2_SYMBOL)) return i;
        }
    }
    // 2. Block opponent's winning move
    for (int i = 0; i < 9; ++i) {
        if (board[i] == ' ') {
            std::vector<char> tempBoard = board;
            tempBoard[i] = P1_SYMBOL;
            if (checkWin(tempBoard, P1_SYMBOL)) return i;
        }
    }
    // 3. Take center
    if (board[4] == ' ') return 4;

    // 4. Take random available corner
    std::vector<int> availableCorners;
    int corners[] = { 0, 2, 6, 8 };
    for (int i : corners) {
        if (board[i] == ' ') availableCorners.push_back(i);
    }
    if (!availableCorners.empty()) {
        return availableCorners[rand() % availableCorners.size()];
    }

    // 5. Take any random available square
    std::vector<int> available;
    for (int i = 0; i < 9; ++i) if (board[i] == ' ') available.push_back(i);
    if (!available.empty()) return available[rand() % available.size()];

    return -1; // Should not happen
}

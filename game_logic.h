#pragma once
#include <vector>

// Game Constants
const char P1_SYMBOL = 'X';
const char P2_SYMBOL = 'O';

// --- Function Declarations ---
// Tells other files that these functions exist and can be called.

bool checkWin(const std::vector<char>& board, char playerSymbol);

bool checkDraw(const std::vector<char>& board);

int findBestMove(std::vector<char> board);
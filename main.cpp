#include "raylib.h"
#include "game_logic.h" // Our game rules
#include "drawing.h"    // Our 3D drawing functions
#include <vector>
#include <string>
#include <ctime>

int main(void)
{
    // --- Initialization ---
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "EchoGrid 3D");
    srand(static_cast<unsigned int>(time(0)));

    // --- Game State Variables ---
    enum GameState { MAIN_MENU, RULES, GAMEPLAY, GAME_OVER };
    GameState currentState = MAIN_MENU;

    std::vector<char> board = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
    int currentPlayer = 1;
    bool vsAI = false;
    std::string winnerText = "";

    // UI state for gameplay
    std::string message = "Roll the dice to see who goes first!";
    std::string subMessage = "Click here to roll";
    bool showingDiceRoll = true;
    bool showingCoinToss = false;
    bool showingPowerMoveChoice = false;
    bool showingConquerChoice = false;
    bool showingDefenseToss = false;
    int defender = 0;
    int conquerTargetIndex = -1;

    // --- 3D Camera Setup ---
    Camera3D camera = { 0 };
    camera.position = { 0.0f, 15.0f, 12.0f };
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // --- Coin Animation Variables ---
    float coinRotation = 0.0f;
    float coinSpinVelocity = 0.0f;
    Vector3 coinPosition = { 8.0f, 2.0f, 0.0f };

    SetTargetFPS(60);

    // --- Main Game Loop ---
    while (!WindowShouldClose())
    {
        // --- Update (Logic) ---
        UpdateCamera(&camera, CAMERA_ORBITAL);

        if (currentState == MAIN_MENU) {
            if (CheckCollisionPointRec(GetMousePosition(), { (float)screenWidth / 2 - 150, 300, 300, 50 }) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                vsAI = false;
                currentState = RULES;
            }
            if (CheckCollisionPointRec(GetMousePosition(), { (float)screenWidth / 2 - 150, 400, 300, 50 }) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                vsAI = true;
                currentState = RULES;
            }
        }
        else if (currentState == RULES) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentState = GAMEPLAY;
            }
        }
        else if (currentState == GAMEPLAY)
        {
            // Animate coin
            if (coinSpinVelocity > 0) {
                coinRotation += coinSpinVelocity;
                coinSpinVelocity *= 0.98f;
                if (coinSpinVelocity < 0.1f) coinSpinVelocity = 0;
            }

            // --- Handle Dice Roll ---
            if (showingDiceRoll && CheckCollisionPointRec(GetMousePosition(), { (float)screenWidth / 2 - 200, (float)screenHeight - 70, 400, 50 }) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                int p1_roll = rand() % 6 + 1;
                int p2_roll = rand() % 6 + 1;

                if (p1_roll > p2_roll) currentPlayer = 1;
                else if (p2_roll > p1_roll) currentPlayer = 2;
                else {
                    message = "It's a tie! Roll again.";
                    continue;
                }

                showingDiceRoll = false;
                showingCoinToss = true;
                message = (currentPlayer == 1 ? "Blue's Turn. " : "Red's Turn. ") + "Call the coin toss!";
                subMessage = "Click Heads or Tails";
            }
            // --- Handle Coin Toss ---
            else if (showingCoinToss) {
                int coinCall = 0;
                if (CheckCollisionPointRec(GetMousePosition(), { (float)screenWidth / 2 - 110, (float)screenHeight - 70, 100, 50 }) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) coinCall = 1;
                if (CheckCollisionPointRec(GetMousePosition(), { (float)screenWidth / 2 + 10, (float)screenHeight - 70, 100, 50 }) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) coinCall = 2;

                if (coinCall > 0) {
                    coinSpinVelocity = 30.0f + (rand() % 20);
                    int coinResult = rand() % 2 + 1;

                    if (coinCall == coinResult) {
                        showingPowerMoveChoice = true;
                        message = "You won the toss! It's a POWER TURN!";
                        subMessage = "Choose to Place or Conquer";
                    }
                    else {
                        showingConquerChoice = false;
                        message = "You lost the toss. Normal turn.";
                        subMessage = "Click an empty square to place your mark.";
                    }
                    showingCoinToss = false;
                }
            }
            // --- Handle Defense Toss ---
            else if (showingDefenseToss) {
                int defenseCall = 0;
                if (CheckCollisionPointRec(GetMousePosition(), { (float)screenWidth / 2 - 110, (float)screenHeight - 70, 100, 50 }) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) defenseCall = 1;
                if (CheckCollisionPointRec(GetMousePosition(), { (float)screenWidth / 2 + 10, (float)screenHeight - 70, 100, 50 }) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) defenseCall = 2;

                if (defenseCall > 0) {
                    coinSpinVelocity = 30.0f + (rand() % 20);
                    int coinResult = rand() % 2 + 1;

                    if (defenseCall == coinResult) {
                        message = "Defense successful! The square is safe.";
                    }
                    else {
                        message = "Defense failed! The square is conquered!";
                        board[conquerTargetIndex] = (currentPlayer == 1 ? P1_SYMBOL : P2_SYMBOL);
                    }

                    showingDefenseToss = false;
                    currentPlayer = (currentPlayer == 1) ? 2 : 1;
                    message = (currentPlayer == 1 ? "Blue's Turn. " : "Red's Turn. ") + "Call the coin toss!";
                    subMessage = "Click Heads or Tails";
                    showingCoinToss = true;
                }
            }
            // --- Handle Player Action ---
            else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Ray ray = GetMouseRay(GetMousePosition(), camera);
                bool moveMade = false;

                if (showingPowerMoveChoice) {
                    if (CheckCollisionPointRec(GetMousePosition(), { (float)screenWidth / 2 - 110, (float)screenHeight - 70, 100, 50 })) { //Place
                        showingPowerMoveChoice = false;
                        showingConquerChoice = false;
                        message = (currentPlayer == 1 ? "Blue's Turn. " : "Red's Turn. ") + "Place your mark.";
                        subMessage = "Click an empty square.";
                    }
                    if (CheckCollisionPointRec(GetMousePosition(), { (float)screenWidth / 2 + 10, (float)screenHeight - 70, 100, 50 })) { //Conquer
                        showingPowerMoveChoice = false;
                        showingConquerChoice = true;
                        message = (currentPlayer == 1 ? "Blue's Turn. " : "Red's Turn. ") + "Choose a square to CONQUER.";
                        subMessage = "Click an opponent's square.";
                    }
                }
                else {
                    for (int i = 0; i < 9; ++i) {
                        Vector3 pos = { (float)(i % 3 - 1) * 4, 0.0f, (float)(i / 3 - 1) * 4 };
                        BoundingBox box = { {pos.x - 1.5f, pos.y - 0.5f, pos.z - 1.5f}, {pos.x + 1.5f, pos.y + 0.5f, pos.z + 1.5f} };
                        if (GetRayCollisionBox(ray, box).hit) {
                            if (showingConquerChoice) {
                                if (board[i] == (currentPlayer == 1 ? P2_SYMBOL : P1_SYMBOL)) {
                                    showingDefenseToss = true;
                                    defender = (currentPlayer == 1 ? 2 : 1);
                                    conquerTargetIndex = i;
                                    message = (defender == 1 ? "Blue" : "Red") + " defends! Call the toss!";
                                    subMessage = "Heads or Tails?";
                                    moveMade = true;
                                }
                                else {
                                    message = "Invalid Target! Turn forfeited.";
                                    moveMade = true;
                                }
                            }
                            else {
                                if (board[i] == ' ') {
                                    board[i] = (currentPlayer == 1 ? P1_SYMBOL : P2_SYMBOL);
                                    moveMade = true;
                                }
                            }
                            break;
                        }
                    }
                }

                if (moveMade && !showingDefenseToss) {
                    if (checkWin(board, (currentPlayer == 1 ? P1_SYMBOL : P2_SYMBOL))) {
                        winnerText = (currentPlayer == 1 ? "BLUE" : "RED") + std::string(" WINS!");
                        currentState = GAME_OVER;
                    }
                    else if (checkDraw(board)) {
                        winnerText = "IT'S A DRAW!";
                        currentState = GAME_OVER;
                    }
                    else {
                        currentPlayer = (currentPlayer == 1) ? 2 : 1;
                        message = (currentPlayer == 1 ? "Blue's Turn. " : "Red's Turn. ") + "Call the coin toss!";
                        subMessage = "Click Heads or Tails";
                        showingCoinToss = true;
                        showingPowerMoveChoice = false;
                        showingConquerChoice = false;
                    }
                }
            }

            // --- AI TURN LOGIC ---
            if (vsAI && currentPlayer == 2 && !showingDiceRoll && !showingCoinToss && !showingPowerMoveChoice && !showingDefenseToss) {
                int move = findBestMove(board);
                if (move != -1) board[move] = P2_SYMBOL;

                if (checkWin(board, P2_SYMBOL)) {
                    winnerText = "RED (AI) WINS!";
                    currentState = GAME_OVER;
                }
                else if (checkDraw(board)) {
                    winnerText = "IT'S A DRAW!";
                    currentState = GAME_OVER;
                }
                else {
                    currentPlayer = 1;
                    message = "Blue's Turn. Call the coin toss!";
                    subMessage = "Click Heads or Tails";
                    showingCoinToss = true;
                }
            }
        }
        else if (currentState == GAME_OVER) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                board.assign(9, ' ');
                currentState = MAIN_MENU;
                showingDiceRoll = true;
                message = "Roll the dice to see who goes first!";
                subMessage = "Click here to roll";
            }
        }

        // --- Draw ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (currentState == MAIN_MENU) {
            DrawText("EchoGrid 3D", screenWidth / 2 - MeasureText("EchoGrid 3D", 80) / 2, 100, 80, DARKGRAY);
            DrawRectangle(screenWidth / 2 - 150, 300, 300, 50, BLUE);
            DrawText("Play vs Human", screenWidth / 2 - MeasureText("Play vs Human", 30) / 2, 310, 30, WHITE);
            DrawRectangle(screenWidth / 2 - 150, 400, 300, 50, RED);
            DrawText("Play vs AI", screenWidth / 2 - MeasureText("Play vs AI", 30) / 2, 410, 30, WHITE);
        }
        else if (currentState == RULES) {
            DrawText("R U L E S", screenWidth / 2 - MeasureText("R U L E S", 40) / 2, 100, 40, GOLD);
            DrawText("- Highest dice roll goes first.", 100, 200, 20, DARKGRAY);
            DrawText("- On your turn, win a coin toss for a POWER TURN.", 100, 230, 20, DARKGRAY);
            DrawText("- POWER TURN: Either PLACE a mark or CONQUER an opponent's mark.", 120, 260, 20, DARKGRAY);
            DrawText("- CONQUER: The defender gets a coin toss. If they win, they save their square!", 140, 290, 20, GREEN);
            DrawText("- NORMAL TURN: You can only PLACE a mark.", 120, 320, 20, DARKGRAY);
            DrawText("- First to get 3 in a row wins!", 100, 350, 20, DARKGRAY);
            DrawText("Click anywhere to start...", screenWidth / 2 - MeasureText("Click anywhere to start...", 20) / 2, 500, 20, LIGHTGRAY);
        }
        else if (currentState == GAMEPLAY || currentState == GAME_OVER) {
            BeginMode3D(camera);
            DrawGrid(10, 4.0f);
            for (int i = 0; i < 9; ++i) {
                Vector3 pos = { (float)(i % 3 - 1) * 4, 0.0f, (float)(i / 3 - 1) * 4 };
                if (board[i] == P1_SYMBOL) DrawX(pos, 2.0f, BLUE);
                if (board[i] == P2_SYMBOL) DrawO(pos, 2.0f, RED);
            }
            rlPushMatrix();
            rlTranslatef(coinPosition.x, coinPosition.y, coinPosition.z);
            rlRotatef(coinRotation, 1.0f, 0.0f, 0.0f);
            DrawCylinder(Vector3Zero(), 1.5f, 1.5f, 0.2f, 32, GOLD);
            DrawCylinderWires(Vector3Zero(), 1.5f, 1.5f, 0.2f, 32, DARKBROWN);
            rlPopMatrix();
            EndMode3D();

            DrawRectangle(0, 0, screenWidth, 100, FADE(WHITE, 0.7f));
            DrawText(message.c_str(), screenWidth / 2 - MeasureText(message.c_str(), 30) / 2, 35, 30, DARKGRAY);
            DrawRectangle(0, screenHeight - 100, screenWidth, 100, FADE(WHITE, 0.7f));

            if (showingDiceRoll) {
                DrawRectangle(screenWidth / 2 - 200, screenHeight - 70, 400, 50, LIME);
                DrawText(subMessage.c_str(), screenWidth / 2 - MeasureText(subMessage.c_str(), 20) / 2, screenHeight - 60, 20, DARKGREEN);
            }
            if (showingCoinToss || showingDefenseToss) {
                DrawRectangle(screenWidth / 2 - 110, screenHeight - 70, 100, 50, GOLD);
                DrawText("Heads", screenWidth / 2 - 110 + 15, screenHeight - 60, 20, DARKBROWN);
                DrawRectangle(screenWidth / 2 + 10, screenHeight - 70, 100, 50, GOLD);
                DrawText("Tails", screenWidth / 2 + 10 + 20, screenHeight - 60, 20, DARKBROWN);
            }
            if (showingPowerMoveChoice) {
                DrawRectangle(screenWidth / 2 - 110, screenHeight - 70, 100, 50, SKYBLUE);
                DrawText("Place", screenWidth / 2 - 110 + 20, screenHeight - 60, 20, DARKBLUE);
                DrawRectangle(screenWidth / 2 + 10, screenHeight - 70, 100, 50, MAROON);
                DrawText("Conquer", screenWidth / 2 + 10 + 10, screenHeight - 60, 20, WHITE);
            }

            if (currentState == GAME_OVER) {
                DrawRectangle(0, 0, screenWidth, screenHeight, FADE(WHITE, 0.8f));
                DrawText(winnerText.c_str(), screenWidth / 2 - MeasureText(winnerText.c_str(), 60) / 2, screenHeight / 2 - 60, 60, BLACK);
                DrawText("Click to return to menu", screenWidth / 2 - MeasureText("Click to return to menu", 20) / 2, screenHeight / 2 + 20, 20, DARKGRAY);
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

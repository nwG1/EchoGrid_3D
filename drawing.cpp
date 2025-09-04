#include "drawing.h"
#include "rlgl.h"

// --- Function Definitions ---
// This is how we actually draw the 3D pieces.

void DrawX(Vector3 position, float size, Color color) {
    rlPushMatrix();
    // Move to the correct position on the board
    rlTranslatef(position.x, position.y, position.z);
    // We rotate the whole drawing space to make the X
    rlRotatef(45, 0, 1, 0);
    DrawCube(Vector3Zero(), size * 1.5f, size / 8, size * 1.5f, color);
    rlRotatef(90, 0, 1, 0);
    DrawCube(Vector3Zero(), size * 1.5f, size / 8, size * 1.5f, color);
    rlPopMatrix();
}

void DrawO(Vector3 position, float size, Color color) {
    // A torus is a donut shape, perfect for an 'O'
    DrawTorus(position, size * 0.5f, size / 8, 16, 32, color);
}

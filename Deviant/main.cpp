#include "raylib.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

int main(int argc, char* argv[])
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Deviant");
    SetTargetFPS(60);

    // Esc or the close button ends the loop
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Deviant", 40, 40, 40, RAYWHITE);
        DrawText("Phase 0: the window opens", 40, 100, 20, GRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

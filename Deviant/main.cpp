#include "raylib.h"
#include "demo/Demo.h"

#include <iostream>
#include <string>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

int main(int argc, char* argv[])
{
    // Demo mode: Deviant.exe --demo <concept> prints to the console, no window
    if (argc > 1)
    {
        std::string flag = argv[1];

        if (flag == "--demo")
        {
            // No concept given
            if (argc < 3)
            {
                printDemoUsage();
                return 1;
            }

            std::string concept = argv[2];

            if (!runDemo(concept))
            {
                std::cout << "Unknown concept: " << concept << "\n";
                printDemoUsage();
                return 1;
            }

            return 0;
        }
    }

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

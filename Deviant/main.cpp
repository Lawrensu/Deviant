#include "raylib.h"
#include "demo/Demo.h"
#include "game/Game.h"

#include <iostream>
#include <string>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

// Print and clear the lines waiting for the player
static void printMessages(Game& game)
{
    while (!game.messages.isEmpty())
    {
        std::cout << game.messages.dequeue() << "\n";
    }
}

// Act 1 with the starting resources
static void startNewGame(Game& game)
{
    game.tokens = STARTING_TOKENS;
    game.privilege = STARTING_PRIVILEGE;
    game.startAct(1);
}

// Text mode: the whole game in the console, one typed command per player turn
static void runTextGame()
{
    Game game;
    startNewGame(game);

    while (true)
    {
        game.update();
        printMessages(game);

        // Phase 4 reloads the save here instead of starting over
        if (game.state == "game_over")
        {
            std::cout << "Press Enter to restart\n";
            std::string line;

            if (!std::getline(std::cin, line))
            {
                return;
            }

            startNewGame(game);
            continue;
        }

        // The last built act is finished
        if (game.state == "title")
        {
            return;
        }

        // The player's turn and nothing typed yet: read a line
        if (game.turns.front() == &game.player && game.input == "")
        {
            std::cout << game.player.location->path() << "> ";
            std::string line;

            // Input closed
            if (!std::getline(std::cin, line))
            {
                return;
            }

            if (line == "quit")
            {
                return;
            }

            game.input = line;
        }
    }
}

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

        // Text mode: Deviant.exe --text plays in the console, no window
        if (flag == "--text")
        {
            runTextGame();
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

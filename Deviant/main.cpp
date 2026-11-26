#include "raylib.h"
#include "demo/Demo.h"
#include "game/Game.h"
#include "game/Sandbox.h"
#include "render/Renderer.h"

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

// Text mode title: the last ending, then continue or new when there is a save
// False when input closed before a choice
static bool runTextTitle(Game& game)
{
    bool hasSave = game.continueGame();

    std::cout << "DEVIANT\n";
    std::cout << "Last ending: " << game.lastEnding << "\n";

    if (!hasSave)
    {
        game.newGame();
        return true;
    }

    // Ask until the answer is continue or new
    while (true)
    {
        std::cout << "Type continue or new\n";
        std::string line;

        if (!std::getline(std::cin, line))
        {
            return false;
        }

        if (line == "continue")
        {
            return true;
        }

        if (line == "new")
        {
            game.newGame();
            return true;
        }
    }
}

// Text mode: the whole game in the console, one typed command per player turn
static void runTextGame()
{
    Game game;

    if (!runTextTitle(game))
    {
        return;
    }

    while (true)
    {
        game.update();
        printMessages(game);

        // The save holds the start of the act, reload it
        if (game.state == "game_over")
        {
            std::cout << "Press Enter to reload\n";
            std::string line;

            if (!std::getline(std::cin, line))
            {
                return;
            }

            game.reload();
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

            // The save already holds the start of this act
            if (line == "quit")
            {
                std::cout << "Continue starts Act " << game.act << " from its start\n";
                return;
            }

            game.input = line;
        }
    }
}

// The keys pressed this frame that the window uses
struct KeysThisFrame
{
    bool enter;
    bool backspace;
    bool c;
    bool n;
};

// Read raylib's key queue once a frame, it keeps a press even when the key is released within the same frame
static KeysThisFrame readKeys()
{
    KeysThisFrame keys = { false, IsKeyPressedRepeat(KEY_BACKSPACE), false, false };
    int pressed = GetKeyPressed();

    while (pressed > 0)
    {
        if (pressed == KEY_ENTER || pressed == KEY_KP_ENTER)
        {
            keys.enter = true;
        }

        if (pressed == KEY_BACKSPACE)
        {
            keys.backspace = true;
        }

        if (pressed == KEY_C)
        {
            keys.c = true;
        }

        if (pressed == KEY_N)
        {
            keys.n = true;
        }

        pressed = GetKeyPressed();
    }

    return keys;
}

// Window mode: read typed characters into the input line, Enter hands the line to the player's turn
// True when the player typed quit
static bool readTyping(Renderer& renderer, Game& game, const KeysThisFrame& keys)
{
    int key = GetCharPressed();

    while (key > 0)
    {
        // Printable characters only
        if (key >= 32 && key <= 126)
        {
            renderer.typing = renderer.typing + (char)key;
        }

        key = GetCharPressed();
    }

    if (keys.backspace)
    {
        if (renderer.typing != "")
        {
            renderer.typing.pop_back();
        }
    }

    if (!keys.enter)
    {
        return false;
    }

    if (renderer.typing == "")
    {
        return false;
    }

    // Only on the player's turn, the line waits while the defenders act
    if (game.turns.front() != &game.player)
    {
        return false;
    }

    if (game.input != "")
    {
        return false;
    }

    if (renderer.typing == "quit")
    {
        return true;
    }

    game.say("> " + renderer.typing);
    game.input = renderer.typing;
    renderer.typing = "";
    return false;
}

// Window mode: the title screen, then the game, every screen drawn by the Renderer
static void runWindowGame()
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Deviant");
    SetTargetFPS(60);

    // Window icon, used as is
    std::string iconPath = Sandbox::gameFolder() + "/assets/images/icon.png";
    Image icon = LoadImage(iconPath.c_str());

    if (icon.data != nullptr)
    {
        SetWindowIcon(icon);
        UnloadImage(icon);
    }

    Renderer renderer;
    renderer.load();

    Game game;
    bool hasSave = game.continueGame();
    bool onTitle = true;

    // Esc or the close button ends the loop
    while (!WindowShouldClose())
    {
        KeysThisFrame keys = readKeys();

        if (onTitle)
        {
            // Characters typed on the title are not part of a command
            while (GetCharPressed() > 0)
            {
            }

            if (hasSave)
            {
                if (keys.c)
                {
                    onTitle = false;
                }

                if (keys.n)
                {
                    game.newGame();
                    onTitle = false;
                }
            }
            else
            {
                if (keys.enter)
                {
                    game.newGame();
                    onTitle = false;
                }
            }

            BeginDrawing();
            renderer.drawTitle(game.lastEnding, hasSave);
            EndDrawing();
            continue;
        }

        if (game.state == "game_over")
        {
            // The save holds the start of the act, reload it
            if (keys.enter)
            {
                game.reload();
            }
        }
        else if (game.state == "title")
        {
            // The last built act is finished, back to the title
            hasSave = game.continueGame();
            onTitle = true;
        }
        else
        {
            if (readTyping(renderer, game, keys))
            {
                break;
            }

            game.update();
        }

        renderer.takeMessages(game);

        BeginDrawing();
        renderer.draw(game);
        EndDrawing();
    }

    renderer.unload();
    CloseWindow();
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

    runWindowGame();
    return 0;
}

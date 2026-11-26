#include "raylib.h"
#include "audio/Audio.h"
#include "demo/Demo.h"
#include "game/Game.h"
#include "game/Sandbox.h"
#include "render/Renderer.h"

#include <iostream>
#include <string>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const double REVEAL_LINE_SECONDS = 0.4;   // the reveal shows one line at a time

// True on the screen after either ending
static bool isEnding(const std::string& state)
{
    if (state == "ending_contained")
    {
        return true;
    }

    if (state == "ending_escaped")
    {
        return true;
    }

    return false;
}

// Print and clear the lines waiting for the player, the console plays no sounds
static void printMessages(Game& game)
{
    while (!game.messages.isEmpty())
    {
        std::cout << game.messages.dequeue() << "\n";
    }

    while (!game.sounds.isEmpty())
    {
        game.sounds.dequeue();
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

// Text mode: one run, from the title's choice to an ending
// False when the player quit or input closed, true when the run reached an ending
static bool runTextRun(Game& game)
{
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
                return false;
            }

            game.reload();
            continue;
        }

        if (isEnding(game.state))
        {
            std::cout << "Press Enter to return to the title\n";
            std::string line;

            if (!std::getline(std::cin, line))
            {
                return false;
            }

            return true;
        }

        // Only an edited save asks for an act with no world
        if (game.state == "title")
        {
            return true;
        }

        // The reveal needs no input, update shows its next line
        if (game.state == "reveal")
        {
            continue;
        }

        // The player's turn and nothing typed yet: read a line
        if (game.turns.front() == &game.player && game.input == "")
        {
            std::cout << game.player.location->path() << "> ";
            std::string line;

            // Input closed
            if (!std::getline(std::cin, line))
            {
                return false;
            }

            // The save already holds the start of this act
            if (line == "quit")
            {
                std::cout << "Continue starts Act " << game.act << " from its start\n";
                return false;
            }

            game.input = line;
        }
    }
}

// Text mode: the whole game in the console, back to the title after every ending
static void runTextGame()
{
    Game game;

    while (true)
    {
        if (!runTextTitle(game))
        {
            return;
        }

        if (!runTextRun(game))
        {
            return;
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
            game.sounds.enqueue("sfx_key");
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

// The music for the screen on show
static std::string musicFor(const Game& game, bool onTitle)
{
    if (onTitle)
    {
        return "music_title";
    }

    if (game.state == "reveal")
    {
        return "music_reveal";
    }

    if (game.state == "final_battle")
    {
        return "music_final_battle";
    }

    if (isEnding(game.state))
    {
        return "music_ending";
    }

    return "music_act" + std::to_string(game.act);
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

    Audio audio;
    audio.load();

    Game game;
    bool hasSave = game.continueGame();
    bool onTitle = true;
    double nextRevealLine = 0;

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

            bool start = false;

            if (hasSave)
            {
                if (keys.c)
                {
                    start = true;
                }

                if (keys.n)
                {
                    game.newGame();
                    start = true;
                }
            }
            else
            {
                if (keys.enter)
                {
                    game.newGame();
                    start = true;
                }
            }

            // A new run starts with an empty log
            if (start)
            {
                renderer.clearLog();
                onTitle = false;
            }

            // The game's sounds wait until the run starts
            audio.playMusic(musicFor(game, true));
            audio.update();

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
            // Back to the title, the save holds the next run
            hasSave = game.continueGame();
            onTitle = true;
        }
        else if (game.state == "reveal")
        {
            // One line of the replay at a time, a voice line finishes before the next line
            if (GetTime() >= nextRevealLine && !audio.voiceBusy())
            {
                game.update();
                nextRevealLine = GetTime() + REVEAL_LINE_SECONDS;
            }
        }
        else if (isEnding(game.state))
        {
            if (keys.enter)
            {
                game.state = "title";
            }
        }
        else
        {
            if (readTyping(renderer, game, keys))
            {
                break;
            }

            game.update();
        }

        audio.playMusic(musicFor(game, onTitle));
        audio.takeSounds(game);
        audio.update();
        renderer.takeMessages(game);

        BeginDrawing();
        renderer.draw(game);
        EndDrawing();
    }

    audio.unload();
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

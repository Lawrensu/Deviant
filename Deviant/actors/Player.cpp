#include "Player.h"
#include "../game/Game.h"
#include "../commands/Parser.h"

Player::Player() : Process("Sol.exe") {}

// Polymorphism: the player's turn runs the typed command
bool Player::takeTurn(Game& game)
{
    // Nothing typed yet, still the player's turn
    if (game.input == "")
    {
        return false;
    }

    std::string typed = game.input;
    game.input = "";

    // Rewind is the invoker's job, not a command
    if (typed == "rewind")
    {
        return game.rewind();
    }

    Parser parser;
    Command* command = parser.parse(typed, game);

    // A view, a mistake or a refused action, still the player's turn
    if (command == nullptr)
    {
        return false;
    }

    return game.perform(command);
}

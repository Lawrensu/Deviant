#include "Player.h"
#include "../game/Game.h"

Player::Player() : Process("Sol.exe") {}

// Polymorphism: the player's turn waits for a typed command
bool Player::takeTurn(Game& game)
{
    // Phase 3: Parser turns the typed input into a command and Game performs it
    // Until then there is never a command, so the turn is not done
    return false;
}

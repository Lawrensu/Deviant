#include "Fable.h"
#include "../game/Game.h"

const int FABLE_ENTRY_DETECTION = 10;   // DRAFT

Fable::Fable() : Defender("Fable"), blockedFragment(nullptr), watching(nullptr) {}

// Polymorphism: Fable's turn follows a fixed rule, so her directories are a planning puzzle
bool Fable::takeTurn(Game& game)
{
    Directory* playerAt = game.player.location;

    // Player outside her directories: nothing is blocked
    if (playerAt == nullptr || !playerAt->guardedByFable)
    {
        watching = nullptr;
        blockedFragment = nullptr;
        return true;
    }

    // Player just entered one of her directories
    if (playerAt != watching)
    {
        game.raiseDetection(FABLE_ENTRY_DETECTION);
        watching = playerAt;
    }

    // Phase 3: read the history backward for the last added fragment and block it
    blockedFragment = nullptr;
    return true;
}

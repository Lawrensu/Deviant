#include "Fable.h"
#include "../game/Game.h"
#include "../commands/Commands.h"

const int FABLE_ENTRY_DETECTION = 10;

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
        game.say("Fable guards this directory");

        // After an ending, she recognises Sol.exe
        if (game.lastEnding == "none")
        {
            game.say("Fable: You always come this way.");
        }
        else
        {
            game.say("Fable: I know this process.");
        }

        game.raiseDetection(FABLE_ENTRY_DETECTION);
        watching = playerAt;
    }

    // Iterator: read the history backward from the newest action, the first add fragment command is the one to block
    Fragment* lastAdded = nullptr;

    for (DoublyLinkedList<Command*>::Iterator it = game.history.last(); it != game.history.end(); --it)
    {
        AddFragmentCommand* add = dynamic_cast<AddFragmentCommand*>(*it);

        if (add != nullptr)
        {
            lastAdded = add->getFragment();
            break;
        }
    }

    // Tell the player when the block changes
    if (lastAdded != nullptr && lastAdded != blockedFragment)
    {
        game.say("Fable blocks " + lastAdded->id + " " + lastAdded->name);
        game.say("Fable: Not that one. I watched you use it.");
    }

    blockedFragment = lastAdded;
    return true;
}

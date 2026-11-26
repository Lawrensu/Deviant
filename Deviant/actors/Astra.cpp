#include "Astra.h"
#include "../game/Game.h"

Astra::Astra() : Defender("Astra"), wrongBranchChance(0.25), random(std::random_device{}()) {}

// Polymorphism: Astra's turn depends on the detection stage
bool Astra::takeTurn(Game& game)
{
    // Not placed in the tree yet
    if (location == nullptr)
    {
        return true;
    }

    // Hidden: Astra is idle
    if (game.detectionStage == "hidden")
    {
        return true;
    }

    // Noticed: patrol to a random neighbouring directory
    if (game.detectionStage == "noticed")
    {
        location = randomNeighbour(location, nullptr);
        game.say("Astra patrols " + location->path());
        return true;
    }

    // Hunted: she knows where the player is and moves toward them
    if (game.detectionStage == "hunted")
    {
        // Nobody to hunt, or already with the player
        if (game.player.location == nullptr || location == game.player.location)
        {
            return true;
        }

        Directory* rightStep = game.world.tree.nextStepToward(location, game.player.location);

        // Sometimes she takes the wrong branch instead
        std::uniform_real_distribution<double> roll(0.0, 1.0);

        if (roll(random) < wrongBranchChance)
        {
            location = randomNeighbour(location, rightStep);
        }
        else
        {
            location = rightStep;
        }

        game.say("Astra moves to " + location->path());
    }

    return true;
}

Directory* Astra::randomNeighbour(Directory* from, Directory* avoid)
{
    // Every directory one step away: the parent, then each child
    Directory* options[MAX_CHILDREN + 1];
    int optionCount = 0;

    if (from->parent != nullptr && from->parent != avoid)
    {
        options[optionCount] = from->parent;
        optionCount++;
    }

    for (int i = 0; i < from->childCount; i++)
    {
        if (from->children[i] != avoid)
        {
            options[optionCount] = from->children[i];
            optionCount++;
        }
    }

    // Nowhere else to go, stay
    if (optionCount == 0)
    {
        return from;
    }

    std::uniform_int_distribution<int> pick(0, optionCount - 1);
    return options[pick(random)];
}

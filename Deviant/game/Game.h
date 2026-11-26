#pragma once

#include <string>

#include "../world/World.h"
#include "../actors/Player.h"
#include "../actors/Fable.h"
#include "../actors/Astra.h"

// The game state, resources and actors
// Phase 2 holds what the actors read and change, Phase 3 adds turns, commands and acts
class Game
{
public:
    std::string state;            // "title", "exploring", "reveal", "final_battle", "game_over", "ending_contained", "ending_escaped"
    int act;
    int tokens;
    int privilege;
    int detection;
    std::string detectionStage;   // "hidden", "noticed", "hunted"

    World world;
    Player player;
    Fable fable;
    Astra astra;

    Game();

    // Add detection and move to the stage it now falls in
    void raiseDetection(int amount);
};

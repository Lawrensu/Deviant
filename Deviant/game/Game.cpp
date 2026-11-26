#include "Game.h"

const int NOTICED_AT = 30;   // DRAFT: detection where the stage becomes noticed
const int HUNTED_AT = 60;    // DRAFT: detection where the stage becomes hunted

Game::Game()
    : state("title"), act(0), tokens(0), privilege(0), detection(0), detectionStage("hidden") {}

void Game::raiseDetection(int amount)
{
    detection = detection + amount;

    // Pick the stage from the thresholds, highest first
    if (detection >= HUNTED_AT)
    {
        detectionStage = "hunted";
    }
    else if (detection >= NOTICED_AT)
    {
        detectionStage = "noticed";
    }
    else
    {
        detectionStage = "hidden";
    }
}

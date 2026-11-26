#pragma once

#include <random>

#include "Defender.h"

// Inheritance: Astra, the hunter, is a Defender
// She acts by detection stage, and while hunting sometimes takes the wrong branch
class Astra : public Defender
{
public:
    double wrongBranchChance;   // 0 to 1, chance of a wrong step while hunting

    Astra();

    // Idle when hidden, patrol when noticed, hunt when hunted
    bool takeTurn(Game& game) override;

private:
    std::mt19937 random;

    // A random directory next to this one (its parent or a child), skipping avoid, this one when there is none
    Directory* randomNeighbour(Directory* from, Directory* avoid);
};

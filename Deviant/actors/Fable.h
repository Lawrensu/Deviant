#pragma once

#include "Defender.h"

struct Fragment;

// Inheritance: Fable, the predictor, is a Defender
// She guards fixed directories and never moves
class Fable : public Defender
{
public:
    Fragment* blockedFragment;   // cannot be added to the chain, nullptr when nothing is blocked

    Fable();

    // Raises detection when the player enters her directory, blocks the last added fragment while they stay (Phase 3)
    bool takeTurn(Game& game) override;

private:
    Directory* watching;         // her directory the player was in last turn, nullptr when outside
};

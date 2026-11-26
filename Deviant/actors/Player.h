#pragma once

#include "Process.h"

// Inheritance: the player, Sol.exe, is a Process
class Player : public Process
{
public:
    Player();

    // Done once the typed command has run
    bool takeTurn(Game& game) override;
};

#pragma once

#include "Process.h"

class Player;

// Inheritance: shared defender behaviour, Fable and Astra derive from it
// Still abstract, each defender overrides takeTurn
class Defender : public Process
{
public:
    Defender(const std::string& name);

    // True when the defender is in the same directory as the player
    bool catches(const Player& player) const;
};

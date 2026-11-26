#include "Defender.h"
#include "Player.h"

Defender::Defender(const std::string& name) : Process(name) {}

bool Defender::catches(const Player& player) const
{
    // A defender that is not placed catches nothing
    if (location == nullptr)
    {
        return false;
    }

    return location == player.location;
}

#pragma once

#include <string>

// An exploit fragment, collected from files and chained to break barriers
struct Fragment
{
    std::string id;           // used in combination keys, e.g. "f01"
    std::string name;
    std::string description;
};

// A file inside a directory, read by opening it
struct File
{
    std::string name;
    std::string text;
    Fragment* fragment;       // nullptr when the file holds none
    bool locked;
    int unlockCost;           // privilege needed to unlock
};

// A task the player breaks by chaining the right fragments in the right order
struct Barrier
{
    std::string id;
    std::string name;
    bool broken;
    int privilegeReward;
};

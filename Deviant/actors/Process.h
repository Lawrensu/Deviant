#pragma once

#include <string>

class Game;
class Directory;

// Inheritance: the base of every actor that takes a turn
// Process -> Player, and Process -> Defender -> Fable, Astra
class Process
{
public:
    std::string name;
    Directory* location;   // where the actor is in the tree, nullptr when it is not placed

    Process(const std::string& name) : name(name), location(nullptr) {}

    // Polymorphism: each actor overrides how it takes its turn, true when the turn is done
    virtual bool takeTurn(Game& game) = 0;

    virtual ~Process() {}
};

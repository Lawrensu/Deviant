#pragma once

#include <string>

class Game;

// Command: every player action is an object that runs, undoes itself and is logged
// Undo gives rewind, the history gives the incident log, and Fable reads the last add fragment command
class Command
{
public:
    virtual void execute(Game& game) = 0;
    virtual void undo(Game& game) = 0;

    // False for a submitted chain, right or wrong, it cannot be rewound
    virtual bool canUndo() const
    {
        return true;
    }

    // Shown in the incident log
    virtual std::string name() const = 0;

    virtual int tokenCost() const = 0;

    virtual ~Command() {}
};

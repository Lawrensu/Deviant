#include "Commands.h"
#include "../game/Game.h"

const int MOVE_TOKENS = 1;
const int OPEN_TOKENS = 1;
const int UNLOCK_TOKENS = 1;
const int ADD_TOKENS = 1;
const int REMOVE_TOKENS = 1;
const int SUBMIT_TOKENS = 2;
const int UNLOCK_DETECTION = 5;
const int WRONG_CHAIN_TOKENS = 3;       // on top of SUBMIT_TOKENS
const int WRONG_CHAIN_DETECTION = 15;


// MoveCommand

MoveCommand::MoveCommand(Directory* target) : target(target), previous(nullptr) {}

void MoveCommand::execute(Game& game)
{
    previous = game.player.location;
    game.player.location = target;
    game.say("Moved to " + target->path());
}

void MoveCommand::undo(Game& game)
{
    game.player.location = previous;
}

std::string MoveCommand::name() const
{
    return "move to " + target->path();
}

int MoveCommand::tokenCost() const
{
    return MOVE_TOKENS;
}


// OpenFileCommand

OpenFileCommand::OpenFileCommand(File* file) : file(file), taken(nullptr) {}

void OpenFileCommand::execute(Game& game)
{
    game.say(file->name + ": " + file->text);

    // Take the fragment, the file no longer holds it
    if (file->fragment != nullptr)
    {
        taken = file->fragment;
        file->fragment = nullptr;
        game.inventory.pushBack(taken);
        game.say("Took fragment " + taken->id + " " + taken->name);
    }
}

void OpenFileCommand::undo(Game& game)
{
    // Take the fragment out of the inventory and put it back in the file
    if (taken != nullptr)
    {
        game.inventory.remove(taken);
        file->fragment = taken;
    }
}

std::string OpenFileCommand::name() const
{
    return "open " + file->name;
}

int OpenFileCommand::tokenCost() const
{
    return OPEN_TOKENS;
}


// UnlockCommand

UnlockCommand::UnlockCommand(Directory* directory, File* file)
    : directory(directory), file(file), privilegeSpent(0) {}

void UnlockCommand::execute(Game& game)
{
    std::string unlocked = "";

    if (directory != nullptr)
    {
        privilegeSpent = directory->unlockCost;
        directory->locked = false;
        unlocked = directory->path();
    }
    else
    {
        privilegeSpent = file->unlockCost;
        file->locked = false;
        unlocked = file->name;
    }

    game.privilege = game.privilege - privilegeSpent;
    game.say("Unlocked " + unlocked + ", spent " + std::to_string(privilegeSpent) + " privilege");

    // Unlocking is risky, it raises detection
    game.raiseDetection(UNLOCK_DETECTION);
}

void UnlockCommand::undo(Game& game)
{
    if (directory != nullptr)
    {
        directory->locked = true;
    }
    else
    {
        file->locked = true;
    }

    game.privilege = game.privilege + privilegeSpent;
}

std::string UnlockCommand::name() const
{
    if (directory != nullptr)
    {
        return "unlock " + directory->path();
    }

    return "unlock " + file->name;
}

int UnlockCommand::tokenCost() const
{
    return UNLOCK_TOKENS;
}


// AddFragmentCommand

AddFragmentCommand::AddFragmentCommand(Fragment* fragment) : fragment(fragment) {}

void AddFragmentCommand::execute(Game& game)
{
    game.inventory.remove(fragment);
    game.chain.push(fragment);
    game.say("Added " + fragment->id + " " + fragment->name + " to the chain");
}

// Pop it off the chain and return it to the inventory
void AddFragmentCommand::undo(Game& game)
{
    game.chain.pop();
    game.inventory.pushBack(fragment);
}

std::string AddFragmentCommand::name() const
{
    return "add " + fragment->id + " " + fragment->name;
}

int AddFragmentCommand::tokenCost() const
{
    return ADD_TOKENS;
}

Fragment* AddFragmentCommand::getFragment() const
{
    return fragment;
}


// RemoveFragmentCommand

RemoveFragmentCommand::RemoveFragmentCommand() : fragment(nullptr) {}

void RemoveFragmentCommand::execute(Game& game)
{
    // Stack: only the last added fragment can come off
    fragment = game.chain.pop();
    game.inventory.pushBack(fragment);
    game.say("Removed " + fragment->id + " " + fragment->name + " from the chain");
}

// Take it out of the inventory and push it back onto the chain
void RemoveFragmentCommand::undo(Game& game)
{
    game.inventory.remove(fragment);
    game.chain.push(fragment);
}

std::string RemoveFragmentCommand::name() const
{
    // The fragment is only known once it has run
    if (fragment == nullptr)
    {
        return "remove";
    }

    return "remove " + fragment->id + " " + fragment->name;
}

int RemoveFragmentCommand::tokenCost() const
{
    return REMOVE_TOKENS;
}


// SubmitChainCommand

SubmitChainCommand::SubmitChainCommand() {}

void SubmitChainCommand::execute(Game& game)
{
    // Build the key bottom to top, the order the fragments were added, e.g. "f01+f02"
    key = "";

    for (int i = 0; i < game.chain.size(); i++)
    {
        if (i > 0)
        {
            key = key + "+";
        }

        key = key + game.chain.at(i)->id;
    }

    // Hash table: one lookup for what this chain breaks
    Barrier** found = game.world.combinations.find(key);
    Barrier* here = game.player.location->barrier;

    // Right chain for this directory's barrier
    if (found != nullptr && *found == here)
    {
        here->broken = true;
        game.privilege = game.privilege + here->privilegeReward;

        // The fragments are used up
        while (!game.chain.isEmpty())
        {
            game.chain.pop();
        }

        result = "broke " + here->id;
        game.say("Barrier broken: " + here->name + ". Privilege +" + std::to_string(here->privilegeReward));
        return;
    }

    // Wrong chain: it costs more tokens and raises detection
    game.tokens = game.tokens - WRONG_CHAIN_TOKENS;
    game.raiseDetection(WRONG_CHAIN_DETECTION);

    // The fragments return to the inventory, in the order they were added
    for (int i = 0; i < game.chain.size(); i++)
    {
        game.inventory.pushBack(game.chain.at(i));
    }

    while (!game.chain.isEmpty())
    {
        game.chain.pop();
    }

    result = "wrong chain";
    game.say("Wrong chain. The fragments return to your inventory");
}

// Never called, canUndo is false
void SubmitChainCommand::undo(Game& game) {}

bool SubmitChainCommand::canUndo() const
{
    return false;
}

std::string SubmitChainCommand::name() const
{
    return "submit " + key + ", " + result;
}

int SubmitChainCommand::tokenCost() const
{
    return SUBMIT_TOKENS;
}

#include "Parser.h"
#include "Commands.h"
#include "../game/Game.h"

Command* Parser::parse(const std::string& input, Game& game)
{
    // Split into a verb and an argument at the first space, e.g. "cd logs"
    std::string verb = input;
    std::string argument = "";
    size_t space = input.find(' ');

    if (space != std::string::npos)
    {
        verb = input.substr(0, space);
        argument = input.substr(space + 1);
    }

    // Views: free, not recorded, still the player's turn
    if (verb == "ls")
    {
        showDirectory(game);
        return nullptr;
    }

    if (verb == "inventory")
    {
        showInventory(game);
        return nullptr;
    }

    if (verb == "chain")
    {
        showChain(game);
        return nullptr;
    }

    if (verb == "status")
    {
        showStatus(game);
        return nullptr;
    }

    if (verb == "help")
    {
        showHelp(game);
        return nullptr;
    }

    // Commands
    if (verb == "cd")
    {
        return parseMove(argument, game);
    }

    if (verb == "open")
    {
        return parseOpen(argument, game);
    }

    if (verb == "unlock")
    {
        return parseUnlock(argument, game);
    }

    if (verb == "add")
    {
        return parseAdd(argument, game);
    }

    if (verb == "remove")
    {
        return parseRemove(game);
    }

    if (verb == "submit")
    {
        return parseSubmit(game);
    }

    game.say("Unknown command: " + input + ". Type help for the list");
    return nullptr;
}

Command* Parser::parseMove(const std::string& argument, Game& game)
{
    Directory* here = game.player.location;

    if (argument == "")
    {
        game.say("Move where? cd <directory> or cd ..");
        return nullptr;
    }

    // Back to the parent
    if (argument == "..")
    {
        if (here->parent == nullptr)
        {
            game.say("Already at the root");
            return nullptr;
        }

        return new MoveCommand(here->parent);
    }

    Directory* target = findChild(here, argument);

    if (target == nullptr)
    {
        game.say("No directory named " + argument + " here");
        return nullptr;
    }

    if (target->locked)
    {
        game.say(argument + " is locked, unlock it with " + std::to_string(target->unlockCost) + " privilege");
        return nullptr;
    }

    return new MoveCommand(target);
}

Command* Parser::parseOpen(const std::string& argument, Game& game)
{
    if (argument == "")
    {
        game.say("Open what? open <file>");
        return nullptr;
    }

    File* file = findFile(game.player.location, argument);

    if (file == nullptr)
    {
        game.say("No file named " + argument + " here");
        return nullptr;
    }

    if (file->locked)
    {
        game.say(argument + " is locked, unlock it with " + std::to_string(file->unlockCost) + " privilege");
        return nullptr;
    }

    return new OpenFileCommand(file);
}

Command* Parser::parseUnlock(const std::string& argument, Game& game)
{
    if (argument == "")
    {
        game.say("Unlock what? unlock <directory or file>");
        return nullptr;
    }

    // A directory or a file in the current directory
    Directory* directory = findChild(game.player.location, argument);
    File* file = nullptr;

    if (directory == nullptr)
    {
        file = findFile(game.player.location, argument);
    }

    if (directory == nullptr && file == nullptr)
    {
        game.say("Nothing named " + argument + " here");
        return nullptr;
    }

    // Read whichever was found
    bool locked = false;
    int cost = 0;

    if (directory != nullptr)
    {
        locked = directory->locked;
        cost = directory->unlockCost;
    }
    else
    {
        locked = file->locked;
        cost = file->unlockCost;
    }

    if (!locked)
    {
        game.say(argument + " is not locked");
        return nullptr;
    }

    if (game.privilege < cost)
    {
        game.say("Unlocking " + argument + " needs " + std::to_string(cost) + " privilege, you have " + std::to_string(game.privilege));
        return nullptr;
    }

    return new UnlockCommand(directory, file);
}

Command* Parser::parseAdd(const std::string& argument, Game& game)
{
    if (argument == "")
    {
        game.say("Add what? add <fragment>");
        return nullptr;
    }

    // Find the fragment in the inventory by id or name
    Fragment* fragment = nullptr;

    for (Fragment* held : game.inventory)
    {
        if (held->id == argument || held->name == argument)
        {
            fragment = held;
        }
    }

    if (fragment == nullptr)
    {
        game.say("No fragment " + argument + " in your inventory");
        return nullptr;
    }

    if (game.chain.isFull())
    {
        game.say("The chain is full, the limit is " + std::to_string(CHAIN_LIMIT));
        return nullptr;
    }

    // Fable blocks the fragment added last while the player is in her directory
    if (fragment == game.fable.blockedFragment)
    {
        game.say("Fable blocks " + fragment->id + " here");   // DRAFT
        return nullptr;
    }

    return new AddFragmentCommand(fragment);
}

Command* Parser::parseRemove(Game& game)
{
    if (game.chain.isEmpty())
    {
        game.say("The chain is empty");
        return nullptr;
    }

    return new RemoveFragmentCommand();
}

Command* Parser::parseSubmit(Game& game)
{
    Barrier* barrier = game.player.location->barrier;

    if (game.chain.isEmpty())
    {
        game.say("The chain is empty, add fragments first");
        return nullptr;
    }

    if (barrier == nullptr)
    {
        game.say("There is no barrier in this directory");
        return nullptr;
    }

    if (barrier->broken)
    {
        game.say(barrier->name + " is already broken");
        return nullptr;
    }

    return new SubmitChainCommand();
}

void Parser::showDirectory(Game& game)
{
    Directory* here = game.player.location;
    game.say(here->path());

    if (here->barrier != nullptr)
    {
        if (here->barrier->broken)
        {
            game.say("  barrier: " + here->barrier->name + " (broken)");
        }
        else
        {
            game.say("  barrier: " + here->barrier->name);
        }
    }

    for (int i = 0; i < here->childCount; i++)
    {
        Directory* child = here->children[i];

        if (child->locked)
        {
            game.say("  " + child->name + "/  (locked, " + std::to_string(child->unlockCost) + " privilege)");
        }
        else
        {
            game.say("  " + child->name + "/");
        }
    }

    for (int i = 0; i < here->fileCount; i++)
    {
        File* file = here->files[i];

        if (file->locked)
        {
            game.say("  " + file->name + "  (locked, " + std::to_string(file->unlockCost) + " privilege)");
        }
        else
        {
            game.say("  " + file->name);
        }
    }
}

void Parser::showInventory(Game& game)
{
    if (game.inventory.size() == 0)
    {
        game.say("Inventory is empty");
        return;
    }

    game.say("Inventory:");

    // Iterator: walk the inventory front to back
    for (Fragment* fragment : game.inventory)
    {
        game.say("  " + fragment->id + " " + fragment->name + ": " + fragment->description);
    }
}

void Parser::showChain(Game& game)
{
    if (game.chain.isEmpty())
    {
        game.say("Chain is empty");
        return;
    }

    // Bottom to top, the order the fragments were added
    std::string line = "Chain:";

    for (int i = 0; i < game.chain.size(); i++)
    {
        line = line + " " + game.chain.at(i)->id;
    }

    game.say(line + "  (" + std::to_string(game.chain.size()) + " of " + std::to_string(CHAIN_LIMIT) + ")");
}

void Parser::showStatus(Game& game)
{
    game.say("Act " + std::to_string(game.act) + ", at " + game.player.location->path());
    game.say("Tokens " + std::to_string(game.tokens) + ", privilege " + std::to_string(game.privilege));
    game.say("Detection " + std::to_string(game.detection) + ", " + game.detectionStage);

    if (game.astra.location != nullptr)
    {
        game.say("Astra is at " + game.astra.location->path());
    }
}

void Parser::showHelp(Game& game)
{
    game.say("Views, free:   ls, inventory, chain, status, help");
    game.say("Actions:       cd <directory>, cd .., open <file>, unlock <name>");
    game.say("Chain:         add <fragment>, remove, submit");
    game.say("Other:         rewind, quit");
}

Directory* Parser::findChild(Directory* directory, const std::string& name)
{
    for (int i = 0; i < directory->childCount; i++)
    {
        if (directory->children[i]->name == name)
        {
            return directory->children[i];
        }
    }

    return nullptr;
}

File* Parser::findFile(Directory* directory, const std::string& name)
{
    for (int i = 0; i < directory->fileCount; i++)
    {
        if (directory->files[i]->name == name)
        {
            return directory->files[i];
        }
    }

    return nullptr;
}

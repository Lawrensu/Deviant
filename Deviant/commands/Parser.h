#pragma once

#include <string>

#include "Command.h"

class Directory;
struct File;
struct Fragment;

// Turns a typed line into a command, the client of the Command pattern
// Every rule is checked here, before any tokens are charged
class Parser
{
public:
    // The command, or nullptr for a view (ls, inventory, chain, status, help), a mistake or a refused action
    // A view prints its output and a refusal says why, both through the game's messages
    Command* parse(const std::string& input, Game& game);

private:
    Command* parseMove(const std::string& argument, Game& game);
    Command* parseOpen(const std::string& argument, Game& game);
    Command* parseUnlock(const std::string& argument, Game& game);
    Command* parseAdd(const std::string& argument, Game& game);
    Command* parseRemove(Game& game);
    Command* parseSubmit(Game& game);

    void showDirectory(Game& game);
    void showInventory(Game& game);
    void showChain(Game& game);
    void showStatus(Game& game);
    void showHelp(Game& game);

    Directory* findChild(Directory* directory, const std::string& name);
    File* findFile(Directory* directory, const std::string& name);
};

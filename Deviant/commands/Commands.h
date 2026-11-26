#pragma once

#include <string>

#include "Command.h"

class Directory;
struct File;
struct Fragment;

// Command: move into a child directory or back to the parent
class MoveCommand : public Command
{
public:
    MoveCommand(Directory* target);
    void execute(Game& game) override;
    void undo(Game& game) override;
    std::string name() const override;
    int tokenCost() const override;

private:
    Directory* target;
    Directory* previous;   // the directory it left
};

// Command: read a file, and take the fragment inside it
class OpenFileCommand : public Command
{
public:
    OpenFileCommand(File* file);
    void execute(Game& game) override;
    void undo(Game& game) override;
    std::string name() const override;
    int tokenCost() const override;

private:
    File* file;
    Fragment* taken;       // the fragment it took, nullptr when the file held none
};

// Command: spend privilege to unlock a directory or a file, one of the two is nullptr
class UnlockCommand : public Command
{
public:
    UnlockCommand(Directory* directory, File* file);
    void execute(Game& game) override;
    void undo(Game& game) override;
    std::string name() const override;
    int tokenCost() const override;

private:
    Directory* directory;
    File* file;
    int privilegeSpent;
};

// Command: move a fragment from the inventory onto the chain
class AddFragmentCommand : public Command
{
public:
    AddFragmentCommand(Fragment* fragment);
    void execute(Game& game) override;
    void undo(Game& game) override;
    std::string name() const override;
    int tokenCost() const override;

    // Read by Fable to find the fragment to block
    Fragment* getFragment() const;

private:
    Fragment* fragment;
};

// Command: take the last added fragment off the chain, back to the inventory
class RemoveFragmentCommand : public Command
{
public:
    RemoveFragmentCommand();
    void execute(Game& game) override;
    void undo(Game& game) override;
    std::string name() const override;
    int tokenCost() const override;

private:
    Fragment* fragment;    // set when it runs, the top of the chain
};

// Command: submit the chain against the barrier of the current directory, cannot be undone
class SubmitChainCommand : public Command
{
public:
    SubmitChainCommand();
    void execute(Game& game) override;
    void undo(Game& game) override;
    bool canUndo() const override;
    std::string name() const override;
    int tokenCost() const override;

private:
    std::string key;       // kept only for the incident log
    std::string result;    // kept only for the incident log
};

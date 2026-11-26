#pragma once

#include <string>

// The game's own folder, the only place it reads and writes
// Files for the player go into files/, beside the executable
class Sandbox
{
public:
    // The folder that holds Deviant.exe, every path the game uses starts here
    static std::string gameFolder();

    // Write a file into files/, false when the name holds a folder or drive, or the file cannot be written
    bool writeFile(const std::string& name, const std::string& text);

    bool exists(const std::string& name) const;
};

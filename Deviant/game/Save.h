#pragma once

#include <string>

class Game;

// The save file, save/deviant.sav beside the executable
// One key=value per line, the last line is a checksum of every line above it
// Written at the start of every act and after every reload of an unedited save, so it holds the start of the current act
class Save
{
public:
    Save();

    // Read the save into the game and start its act, false when there is no save
    // An edited or broken save still loads, wasEdited() then returns true
    bool load(Game& game);

    // False when the file cannot be written, for example locked or read only
    bool write(const Game& game);

    // The last load failed its edit check: a wrong or missing checksum, or a value that does not fit the game
    bool wasEdited() const;

private:
    bool edited;

    static std::string path();
    static int checksum(const std::string& text);
};

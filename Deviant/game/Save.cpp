#include "Save.h"
#include "Game.h"
#include "Sandbox.h"

#include <filesystem>
#include <fstream>

const int SAVE_VERSION = 1;
const int CHECKSUM_MODULUS = 100000;

// A whole number, optionally negative, false when the text is anything else
static bool readNumber(const std::string& text, int& number)
{
    if (text == "" || text == "-")
    {
        return false;
    }

    int start = 0;
    bool negative = false;

    if (text[0] == '-')
    {
        negative = true;
        start = 1;
    }

    int result = 0;

    for (int i = start; i < (int)text.size(); i++)
    {
        if (text[i] < '0' || text[i] > '9')
        {
            return false;
        }

        result = result * 10 + (text[i] - '0');
    }

    if (negative)
    {
        result = -result;
    }

    number = result;
    return true;
}

// The fragment's file, searched in pre order, nullptr when no file holds it
static File* fileHolding(DirectoryTree& tree, Fragment* fragment)
{
    for (Directory* directory : tree)
    {
        for (int i = 0; i < directory->fileCount; i++)
        {
            if (directory->files[i]->fragment == fragment)
            {
                return directory->files[i];
            }
        }
    }

    return nullptr;
}

Save::Save() : edited(false) {}

std::string Save::path()
{
    return (std::filesystem::path(Sandbox::gameFolder()) / "save" / "deviant.sav").string();
}

// Polynomial over every character, h = h * 31 + character, the same idea as the hash table's hash
int Save::checksum(const std::string& text)
{
    unsigned int h = 0;

    for (char c : text)
    {
        h = h * 31 + (unsigned char)c;
    }

    return (int)(h % CHECKSUM_MODULUS);
}

bool Save::write(const Game& game)
{
    std::string text = "";
    text = text + "version=" + std::to_string(SAVE_VERSION) + "\n";
    text = text + "act=" + std::to_string(game.act) + "\n";
    text = text + "location=" + game.player.location->path() + "\n";
    text = text + "tokens=" + std::to_string(game.tokens) + "\n";
    text = text + "privilege=" + std::to_string(game.privilege) + "\n";
    text = text + "detection=" + std::to_string(game.detection) + "\n";

    // Fragment ids, comma separated, in inventory order
    std::string inventory = "";

    for (Fragment* fragment : game.inventory)
    {
        if (inventory != "")
        {
            inventory = inventory + ",";
        }

        inventory = inventory + fragment->id;
    }

    text = text + "inventory=" + inventory + "\n";
    text = text + "reload_count=" + std::to_string(game.reloadCount) + "\n";
    text = text + "last_ending=" + game.lastEnding + "\n";

    // Create save/ when it is missing
    std::filesystem::path file(path());
    std::error_code error;
    std::filesystem::create_directories(file.parent_path(), error);

    if (error)
    {
        return false;
    }

    std::ofstream output(file);

    // Locked or read only
    if (!output.is_open())
    {
        return false;
    }

    output << text << "checksum=" << checksum(text) << "\n";
    return output.good();
}

bool Save::load(Game& game)
{
    std::ifstream input(path());

    if (!input.is_open())
    {
        return false;
    }

    edited = false;

    // Defaults for keys that are missing, a missing key also breaks the checksum
    int act = 1;
    std::string location = "/";
    int tokens = STARTING_TOKENS;
    int privilege = STARTING_PRIVILEGE;
    int detection = 0;
    std::string inventory = "";
    int reloadCount = 0;
    std::string lastEnding = "none";
    bool foundAct = false;
    bool foundChecksum = false;
    int storedChecksum = 0;
    std::string linesAbove = "";

    std::string line;

    while (std::getline(input, line))
    {
        size_t equals = line.find('=');

        // Not a key=value line
        if (equals == std::string::npos)
        {
            edited = true;
            continue;
        }

        std::string key = line.substr(0, equals);
        std::string value = line.substr(equals + 1);

        // The checksum covers every line above it
        if (key == "checksum")
        {
            foundChecksum = readNumber(value, storedChecksum);
            break;
        }

        linesAbove = linesAbove + line + "\n";

        if (key == "act")
        {
            foundAct = readNumber(value, act);
        }
        else if (key == "location")
        {
            location = value;
        }
        else if (key == "tokens")
        {
            if (!readNumber(value, tokens))
            {
                edited = true;
            }
        }
        else if (key == "privilege")
        {
            if (!readNumber(value, privilege))
            {
                edited = true;
            }
        }
        else if (key == "detection")
        {
            if (!readNumber(value, detection))
            {
                edited = true;
            }
        }
        else if (key == "inventory")
        {
            inventory = value;
        }
        else if (key == "reload_count")
        {
            if (!readNumber(value, reloadCount))
            {
                edited = true;
            }
        }
        else if (key == "last_ending")
        {
            lastEnding = value;
        }
    }

    // Edit check: a missing or wrong checksum, or an act that cannot be read
    if (!foundChecksum)
    {
        edited = true;
    }
    else if (checksum(linesAbove) != storedChecksum)
    {
        edited = true;
    }

    if (!foundAct)
    {
        edited = true;
        act = 1;
    }

    if (act < 1)
    {
        edited = true;
        act = 1;
    }

    // Apply: counters and resources first, then the act, which builds its world
    game.reloadCount = reloadCount;
    game.lastEnding = lastEnding;
    game.tokens = tokens;
    game.privilege = privilege;
    game.startAct(act);

    // An act that is not built yet falls back to Act 1
    if (game.state != "exploring")
    {
        edited = true;
        game.startAct(1);
    }

    // Place the player, an unknown path stays at the root
    Directory* saved = game.world.tree.find(location);

    if (saved == nullptr)
    {
        edited = true;
    }
    else
    {
        game.player.location = saved;
    }

    // Move each saved fragment out of its file into the inventory
    size_t start = 0;

    while (start < inventory.size())
    {
        size_t comma = inventory.find(',', start);

        if (comma == std::string::npos)
        {
            comma = inventory.size();
        }

        std::string id = inventory.substr(start, comma - start);
        start = comma + 1;

        Fragment* fragment = nullptr;

        for (Fragment* candidate : game.world.fragments)
        {
            if (candidate->id == id)
            {
                fragment = candidate;
            }
        }

        // Not a fragment of this act
        if (fragment == nullptr)
        {
            edited = true;
            continue;
        }

        File* file = fileHolding(game.world.tree, fragment);

        if (file != nullptr)
        {
            file->fragment = nullptr;
        }

        game.inventory.pushBack(fragment);
    }

    if (detection > 0)
    {
        game.raiseDetection(detection);
    }

    return true;
}

bool Save::wasEdited() const
{
    return edited;
}

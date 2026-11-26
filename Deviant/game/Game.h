#pragma once

#include <string>

#include "../world/World.h"
#include "../actors/Player.h"
#include "../actors/Fable.h"
#include "../actors/Astra.h"
#include "../structures/SinglyLinkedList.h"
#include "../structures/DoublyLinkedList.h"
#include "../structures/Stack.h"
#include "../structures/Queue.h"
#include "Save.h"
#include "Sandbox.h"

class Command;

const int CHAIN_LIMIT = 4;
const int STARTING_TOKENS = 60;
const int STARTING_PRIVILEGE = 0;

// The game state, resources, actors and turn loop
// Invoker of the Command pattern: perform runs a command, rewind undoes the last one
class Game
{
public:
    std::string state;            // "title", "exploring", "reveal", "final_battle", "game_over", "ending_contained", "ending_escaped"
    int act;
    int tokens;
    int privilege;
    int detection;
    std::string detectionStage;   // "hidden", "noticed", "hunted"

    World world;
    Player player;
    Fable fable;
    Astra astra;

    SinglyLinkedList<Fragment*> inventory;
    Stack<Fragment*, CHAIN_LIMIT> chain;
    DoublyLinkedList<Command*> history;   // every action of the act, owns the commands
    Queue<Process*> turns;
    Queue<std::string> messages;          // lines waiting to be shown to the player
    std::string input;                    // the line the player typed, empty while waiting

    Save save;
    Sandbox sandbox;
    int reloadCount;                      // reloads after a game over, across every run
    std::string lastEnding;               // "none", "contained" or "escaped", the next run changes after an ending

    Game();

    // Act 1 with the starting resources, then save
    void newGame();

    // Load the save, false when there is none, an edited save starts the player hunted
    bool continueGame();

    // After a game over: load the save from the start of the act, count the reload and comment on it
    void reload();

    // Frees the commands left in the history
    ~Game();

    // Charge, execute, record, false when refused
    bool perform(Command* command);

    // Undo the last action, false when refused
    bool rewind();

    // Add detection, move to the stage it falls in, and tell the player when the stage changes
    void raiseDetection(int amount);

    // One step: the front actor takes its turn
    void update();

    // Build the act and reset what belongs to the last one
    void startAct(int act);

    // Add a line for the player
    void say(const std::string& text);

private:
    void writeSave();   // tells the player when the save cannot be written
    void clearHistory();
    bool actComplete();
    void gameOver(const std::string& reason);
};

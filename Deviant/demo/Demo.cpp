#include "Demo.h"

#include <iostream>
#include <string>

#include "../structures/SinglyLinkedList.h"
#include "../structures/DoublyLinkedList.h"
#include "../structures/Stack.h"
#include "../structures/Queue.h"
#include "../structures/HashTable.h"
#include "../game/Game.h"

// Every concept with a demo, and the phase that builds it
const int DEMO_COUNT = 10;

const char* DEMO_NAMES[DEMO_COUNT] = {
    "inheritance",
    "polymorphism",
    "hash_table",
    "singly_linked_list",
    "doubly_linked_list",
    "stack",
    "queue",
    "tree",
    "iterator",
    "command"
};

const int DEMO_PHASES[DEMO_COUNT] = { 2, 2, 1, 1, 1, 1, 1, 2, 1, 3 };

// Prints a singly linked list and its size on one indented line
static void printSinglyList(const SinglyLinkedList<std::string>& list)
{
    std::cout << "  list:";

    for (SinglyLinkedList<std::string>::Iterator it = list.begin(); it != list.end(); ++it)
    {
        std::cout << " " << *it;
    }

    std::cout << "  (size " << list.size() << ")\n";
}

// Prints a doubly linked list front to back, one action per entry
static void printActionsForward(const DoublyLinkedList<std::string>& history)
{
    std::cout << "  forward:";

    for (DoublyLinkedList<std::string>::Iterator it = history.begin(); it != history.end(); ++it)
    {
        std::cout << " [" << *it << "]";
    }

    std::cout << "  (size " << history.size() << ")\n";
}

// Prints the stack from bottom to top using at()
static void printChain(const Stack<std::string, 3>& chain)
{
    std::cout << "  chain (bottom to top):";

    for (int i = 0; i < chain.size(); i++)
    {
        std::cout << " " << chain.at(i);
    }

    std::cout << "  (size " << chain.size() << ")\n";
}

// Prints the turn order front to back without an iterator
static void printTurnOrder(Queue<std::string>& turnOrder)
{
    std::cout << "  order (front to back):";

    // Dequeue and re-enqueue every actor once, so the order ends up unchanged
    int actorCount = turnOrder.size();

    for (int i = 0; i < actorCount; i++)
    {
        std::string actor = turnOrder.dequeue();
        std::cout << " " << actor;
        turnOrder.enqueue(actor);
    }

    std::cout << "  (size " << turnOrder.size() << ")\n";
}

// One round: the front actor takes its turn, leaves, and rejoins at the back
static void playTurnRound(Queue<std::string>& turnOrder, int roundNumber)
{
    std::string actor = turnOrder.front();
    std::cout << "  round " << roundNumber << ": " << actor << " takes its turn, then rejoins at the back\n";

    turnOrder.dequeue();
    turnOrder.enqueue(actor);
    printTurnOrder(turnOrder);
}

// Prints a hash table lookup result, or nullptr when the key is missing
static void printFindResult(HashTable<std::string>& table, const std::string& key)
{
    std::string* found = table.find(key);

    if (found != nullptr)
    {
        std::cout << "  find " << key << " returned " << *found << "\n";
    }
    else
    {
        std::cout << "  find " << key << " returned nullptr\n";
    }
}

// How many parent links lie between a directory and the root
static int directoryDepth(const Directory* directory)
{
    int depth = 0;
    const Directory* current = directory;

    while (current->parent != nullptr)
    {
        depth++;
        current = current->parent;
    }

    return depth;
}

// The actor's directory path, or "not placed" when it has no directory yet
static std::string locationText(const Process& actor)
{
    if (actor.location == nullptr)
    {
        return "not placed";
    }

    return actor.location->path();
}

// One full round: every actor in the queue takes its turn through a Process pointer
static void playRound(Queue<Process*>& turns, Game& game)
{
    int actorCount = turns.size();

    for (int i = 0; i < actorCount; i++)
    {
        Process* actor = turns.dequeue();
        bool done = actor->takeTurn(game);

        std::cout << "  " << actor->name << " takeTurn returned " << done << ", ";

        if (actor == &game.player)
        {
            std::cout << "still waiting for a typed command, the Parser adds it in Phase 3\n";
        }
        else if (actor == &game.fable)
        {
            std::cout << "detection " << game.detection << ", stage " << game.detectionStage << ", blockedFragment ";

            if (game.fable.blockedFragment != nullptr)
            {
                std::cout << game.fable.blockedFragment->id << "\n";
            }
            else
            {
                std::cout << "none\n";
            }
        }
        else
        {
            std::cout << "now at " << locationText(*actor) << "\n";
        }

        turns.enqueue(actor);
    }
}

// Singly linked list demo: add at the back, search, remove from middle, head and tail
static void demoSinglyLinkedList()
{
    std::cout << "=== Singly linked list demo ===\n";
    std::cout << "Used for: The fragment inventory, in the order fragments were taken\n\n";

    SinglyLinkedList<std::string> inventory;
    int step = 1;

    std::cout << "Step " << step++ << ": pushBack f01\n";
    inventory.pushBack("f01");
    printSinglyList(inventory);

    std::cout << "Step " << step++ << ": pushBack f02\n";
    inventory.pushBack("f02");
    printSinglyList(inventory);

    std::cout << "Step " << step++ << ": pushBack f03\n";
    inventory.pushBack("f03");
    printSinglyList(inventory);

    std::cout << "Step " << step++ << ": pushBack f04\n";
    inventory.pushBack("f04");
    printSinglyList(inventory);

    std::cout << "Step " << step++ << ": contains f03\n";
    bool hasF03 = inventory.contains("f03");
    std::cout << "  contains f03 returned " << hasF03 << "\n";

    std::cout << "Step " << step++ << ": contains f99\n";
    bool hasF99 = inventory.contains("f99");
    std::cout << "  contains f99 returned " << hasF99 << "\n";

    std::cout << "Step " << step++ << ": remove f02 (middle)\n";
    bool removedMiddle = inventory.remove("f02");
    std::cout << "  remove f02 returned " << removedMiddle << "\n";
    printSinglyList(inventory);

    std::cout << "Step " << step++ << ": remove f01 (head)\n";
    bool removedHead = inventory.remove("f01");
    std::cout << "  remove f01 returned " << removedHead << "\n";
    printSinglyList(inventory);

    std::cout << "Step " << step++ << ": remove f04 (tail)\n";
    bool removedTail = inventory.remove("f04");
    std::cout << "  remove f04 returned " << removedTail << "\n";
    printSinglyList(inventory);

    // After the old tail is gone, a new pushBack must link to the new tail
    std::cout << "Step " << step++ << ": pushBack f05 (proves the tail was updated)\n";
    inventory.pushBack("f05");
    printSinglyList(inventory);

    std::cout << "Step " << step++ << ": remove f99 (not in the list)\n";
    bool removedMissing = inventory.remove("f99");
    std::cout << "  remove f99 returned " << removedMissing << "\n";
    printSinglyList(inventory);
}

// Doubly linked list demo: read both ways, then rewind from the tail
static void demoDoublyLinkedList()
{
    std::cout << "=== Doubly linked list demo ===\n";
    std::cout << "Used for: The action history, rewound from the tail and replayed from the head\n\n";

    DoublyLinkedList<std::string> history;
    int step = 1;

    std::cout << "Step " << step++ << ": pushBack \"move logs\"\n";
    history.pushBack("move logs");
    printActionsForward(history);

    std::cout << "Step " << step++ << ": pushBack \"open notes.txt\"\n";
    history.pushBack("open notes.txt");
    printActionsForward(history);

    std::cout << "Step " << step++ << ": pushBack \"add f01\"\n";
    history.pushBack("add f01");
    printActionsForward(history);

    std::cout << "Step " << step++ << ": pushBack \"add f02\"\n";
    history.pushBack("add f02");
    printActionsForward(history);

    std::cout << "Step " << step++ << ": walk forward from begin() with ++\n";
    printActionsForward(history);

    // last() is the tail, -- moves toward the head, end() is past the head
    std::cout << "Step " << step++ << ": walk backward from last() with -- until end()\n";
    std::cout << "  backward:";

    for (DoublyLinkedList<std::string>::Iterator it = history.last(); it != history.end(); --it)
    {
        std::cout << " [" << *it << "]";
    }

    std::cout << "\n";

    std::cout << "Step " << step++ << ": back()\n";
    std::cout << "  back returned " << history.back() << "\n";

    std::cout << "Step " << step++ << ": popBack (rewind)\n";
    std::string firstRewind = history.popBack();
    std::cout << "  rewound: " << firstRewind << "\n";
    printActionsForward(history);

    std::cout << "Step " << step++ << ": popBack (rewind)\n";
    std::string secondRewind = history.popBack();
    std::cout << "  rewound: " << secondRewind << "\n";
    printActionsForward(history);

    std::cout << "Step " << step++ << ": walk forward again from begin()\n";
    printActionsForward(history);

    std::cout << "Step " << step++ << ": size\n";
    std::cout << "  size returned " << history.size() << "\n";
}

// Stack demo: push until full, read bottom to top, pop in reverse
static void demoStack()
{
    std::cout << "=== Stack demo ===\n";
    std::cout << "Used for: The fragment chain, the last fragment added is the first removed\n";
    std::cout << "Capacity is 3, the chain limit\n\n";

    Stack<std::string, 3> chain;
    int step = 1;

    std::cout << "Step " << step++ << ": push f01\n";
    bool pushedFirst = chain.push("f01");
    std::cout << "  push returned " << pushedFirst << "\n";
    printChain(chain);

    std::cout << "Step " << step++ << ": push f02\n";
    bool pushedSecond = chain.push("f02");
    std::cout << "  push returned " << pushedSecond << "\n";
    printChain(chain);

    std::cout << "Step " << step++ << ": push f03\n";
    bool pushedThird = chain.push("f03");
    std::cout << "  push returned " << pushedThird << "\n";
    printChain(chain);

    std::cout << "Step " << step++ << ": push f04 (refused, the chain is full)\n";
    bool pushedFourth = chain.push("f04");
    std::cout << "  push returned " << pushedFourth << "\n";
    std::cout << "  isFull returned " << chain.isFull() << "\n";
    printChain(chain);

    std::cout << "Step " << step++ << ": top\n";
    std::cout << "  top returned " << chain.top() << "\n";

    // Read from the bottom up so the key keeps the order the fragments were added
    std::cout << "Step " << step++ << ": build the combination key from at(0) to at(size-1)\n";
    std::string combinationKey = "";

    for (int i = 0; i < chain.size(); i++)
    {
        if (i > 0)
        {
            combinationKey = combinationKey + "+";
        }

        combinationKey = combinationKey + chain.at(i);
    }

    std::cout << "  key: " << combinationKey << "\n";

    std::cout << "Step " << step++ << ": pop\n";
    std::string firstPopped = chain.pop();
    std::cout << "  pop returned " << firstPopped << "\n";
    printChain(chain);

    std::cout << "Step " << step++ << ": pop until isEmpty is true\n";

    while (!chain.isEmpty())
    {
        std::string popped = chain.pop();
        std::cout << "  pop returned " << popped << "\n";
        printChain(chain);
    }

    std::cout << "  isEmpty returned " << chain.isEmpty() << "\n";
}

// Queue demo: the front actor takes a turn and rejoins at the back
static void demoQueue()
{
    std::cout << "=== Queue demo ===\n";
    std::cout << "Used for: Turn order, the actor at the front takes its turn, then rejoins at the back\n\n";

    Queue<std::string> turnOrder;

    std::cout << "Step 1: enqueue Player\n";
    turnOrder.enqueue("Player");
    printTurnOrder(turnOrder);

    std::cout << "Step 2: enqueue Fable\n";
    turnOrder.enqueue("Fable");
    printTurnOrder(turnOrder);

    std::cout << "Step 3: three rounds\n";
    playTurnRound(turnOrder, 1);
    playTurnRound(turnOrder, 2);
    playTurnRound(turnOrder, 3);

    std::cout << "Step 4: Astra joins at the back\n";
    turnOrder.enqueue("Astra");
    printTurnOrder(turnOrder);

    std::cout << "Step 5: two more rounds\n";
    playTurnRound(turnOrder, 4);
    playTurnRound(turnOrder, 5);
}

// Hash table demo: chaining in buckets, a rehash past load factor 0.75, find, replace, remove
static void demoHashTable()
{
    std::cout << "=== Hash table demo ===\n";
    std::cout << "Used for: Combination lookup, a chain's fragment ids in order to the barrier it breaks\n\n";

    HashTable<std::string> barriers;
    int step = 1;

    std::cout << "Step " << step++ << ": insert f03+f07+f01 -> b01\n";
    barriers.insert("f03+f07+f01", "b01");
    barriers.print();

    std::cout << "Step " << step++ << ": insert f01+f02+f03 -> b02\n";
    barriers.insert("f01+f02+f03", "b02");
    barriers.print();

    std::cout << "Step " << step++ << ": insert f02+f05 -> b03\n";
    barriers.insert("f02+f05", "b03");
    barriers.print();

    std::cout << "Step " << step++ << ": insert f04+f06 -> b04\n";
    barriers.insert("f04+f06", "b04");
    barriers.print();

    // This key lands in the same bucket as f04+f06, so that bucket chains two keys
    std::cout << "Step " << step++ << ": insert f08+f02 -> b05 (same bucket as f04+f06, a chain of 2)\n";
    barriers.insert("f08+f02", "b05");
    barriers.print();

    std::cout << "Step " << step++ << ": insert f05+f01+f02 -> b06\n";
    barriers.insert("f05+f01+f02", "b06");
    barriers.print();

    std::cout << "Step " << step++ << ": insert f06+f09+f04 -> b07\n";
    barriers.insert("f06+f09+f04", "b07");
    std::cout << "  7 entries in 8 buckets is load factor 0.875, past 0.75, so the buckets doubled from 8 to 16\n";
    barriers.print();

    std::cout << "Step " << step++ << ": find f03+f07+f01\n";
    printFindResult(barriers, "f03+f07+f01");

    std::cout << "Step " << step++ << ": find f07+f03+f01 (same fragments, wrong order)\n";
    printFindResult(barriers, "f07+f03+f01");
    std::cout << "  order matters, a different order is a different key\n";

    std::cout << "Step " << step++ << ": insert f03+f07+f01 -> b08 (key exists, value is replaced)\n";
    barriers.insert("f03+f07+f01", "b08");
    printFindResult(barriers, "f03+f07+f01");

    std::cout << "Step " << step++ << ": remove f02+f05\n";
    bool removedExisting = barriers.remove("f02+f05");
    std::cout << "  remove f02+f05 returned " << removedExisting << "\n";
    printFindResult(barriers, "f02+f05");

    std::cout << "Step " << step++ << ": remove f99+f98 (not in the table)\n";
    bool removedMissing = barriers.remove("f99+f98");
    std::cout << "  remove f99+f98 returned " << removedMissing << "\n";

    std::cout << "Step " << step++ << ": print the final table\n";
    barriers.print();
}

// Tree demo: pre order walk, paths, search, and Astra's route through the parent links
static void demoTree()
{
    std::cout << "=== Tree demo ===\n";
    std::cout << "Used for: The directory tree the player explores and Astra hunts through\n\n";

    Game game;
    game.world.buildAct(1);
    int step = 1;

    std::cout << "Step " << step++ << ": walk the whole tree in pre order, a parent before its children\n";

    for (Directory* directory : game.world.tree)
    {
        // Two spaces per level, counted through the parent links
        std::cout << "  ";

        for (int i = 0; i < directoryDepth(directory); i++)
        {
            std::cout << "  ";
        }

        std::cout << directory->name;

        if (directory->locked)
        {
            std::cout << " [locked, cost " << directory->unlockCost << "]";
        }

        if (directory->barrier != nullptr)
        {
            std::cout << " [barrier " << directory->barrier->id << "]";
        }

        if (directory->fileCount > 0)
        {
            std::cout << "  files:";
        }

        for (int i = 0; i < directory->fileCount; i++)
        {
            std::cout << " " << directory->files[i]->name;
        }

        std::cout << "\n";
    }

    std::cout << "Step " << step++ << ": path() of a few directories\n";
    Directory* root = game.world.tree.root;
    Directory* tasks = game.world.tree.find("/sandbox/tasks");
    Directory* task02 = game.world.tree.find("/sandbox/tasks/task_02");
    Directory* tools = game.world.tree.find("/sandbox/tools");
    Directory* logs = game.world.tree.find("/sandbox/logs");
    std::cout << "  root path is " << root->path() << "\n";
    std::cout << "  tasks path is " << tasks->path() << "\n";
    std::cout << "  task_02 path is " << task02->path() << "\n";

    std::cout << "Step " << step++ << ": find /sandbox/logs\n";
    Directory* foundLogs = game.world.tree.find("/sandbox/logs");

    if (foundLogs != nullptr)
    {
        std::cout << "  find returned a directory, path " << foundLogs->path() << "\n";
    }
    else
    {
        std::cout << "  find returned nullptr\n";
    }

    std::cout << "Step " << step++ << ": find /sandbox/missing\n";
    Directory* foundMissing = game.world.tree.find("/sandbox/missing");

    if (foundMissing != nullptr)
    {
        std::cout << "  find returned a directory, path " << foundMissing->path() << "\n";
    }
    else
    {
        std::cout << "  find returned nullptr\n";
    }

    std::cout << "Step " << step++ << ": Astra's route with nextStepToward, from /sandbox/tools to /sandbox/tasks/task_02\n";
    Directory* position = tools;

    while (position != task02)
    {
        position = game.world.tree.nextStepToward(position, task02);
        std::cout << "  step to " << position->path() << "\n";
    }

    std::cout << "  arrived\n";

    std::cout << "Step " << step++ << ": Astra's route, from /sandbox/tasks/task_02 to /sandbox/logs\n";
    position = task02;

    while (position != logs)
    {
        position = game.world.tree.nextStepToward(position, logs);
        std::cout << "  step to " << position->path() << "\n";
    }

    std::cout << "  arrived\n";
    std::cout << "  The route goes up through the parent links until a common ancestor, then down\n";
}

// Inheritance demo: one Process base class, shared members, shared Defender behaviour, derived only members
static void demoInheritance()
{
    std::cout << "=== Inheritance demo ===\n";
    std::cout << "Used for: Every actor derives from one Process base class\n\n";

    Game game;
    game.world.buildAct(1);
    int step = 1;

    std::cout << "Step " << step++ << ": the class hierarchy\n";
    std::cout << "  Process -> Player\n";
    std::cout << "  Process -> Defender -> Fable, Astra\n";

    game.player.location = game.world.tree.find("/sandbox/tasks");
    game.astra.location = game.world.tree.find("/sandbox/logs");

    std::cout << "Step " << step++ << ": name and location are Process members, read through a Process pointer\n";
    std::cout << "  player placed at /sandbox/tasks, Astra at /sandbox/logs, Fable not placed\n";
    Process* actors[3] = { &game.player, &game.fable, &game.astra };

    for (int i = 0; i < 3; i++)
    {
        std::cout << "  name " << actors[i]->name << ", location " << locationText(*actors[i]) << "\n";
    }

    std::cout << "Step " << step++ << ": catches is written once in Defender, Fable and Astra both use it\n";
    Defender* defenders[2] = { &game.fable, &game.astra };

    for (int i = 0; i < 2; i++)
    {
        std::cout << "  " << defenders[i]->name << " catches the player: " << defenders[i]->catches(game.player) << "\n";
    }

    std::cout << "Step " << step++ << ": move Astra to the player's directory, call catches again\n";
    game.astra.location = game.player.location;

    for (int i = 0; i < 2; i++)
    {
        std::cout << "  " << defenders[i]->name << " catches the player: " << defenders[i]->catches(game.player) << "\n";
    }

    std::cout << "  Fable is not placed, so she catches nothing\n";

    std::cout << "Step " << step++ << ": members only a derived class has\n";
    std::cout << "  Fable blockedFragment: ";

    if (game.fable.blockedFragment != nullptr)
    {
        std::cout << game.fable.blockedFragment->id << "\n";
    }
    else
    {
        std::cout << "none\n";
    }

    std::cout << "  Astra wrongBranchChance: " << game.astra.wrongBranchChance << "\n";
}

// Polymorphism demo: the turn queue calls takeTurn through Process pointers, each actor answers its own way
static void demoPolymorphism()
{
    std::cout << "=== Polymorphism demo ===\n";
    std::cout << "Used for: The turn queue calls takeTurn on each actor without knowing which one it is\n\n";

    Game game;
    game.world.buildAct(1);
    int step = 1;

    game.player.location = game.world.tree.find("/sandbox/tasks");
    game.astra.location = game.world.tree.find("/sandbox/logs");
    Directory* task02 = game.world.tree.find("/sandbox/tasks/task_02");

    // Demo setup only, the real guarded directories come with the later acts
    task02->guardedByFable = true;

    Queue<Process*> turns;
    turns.enqueue(&game.player);
    turns.enqueue(&game.fable);
    turns.enqueue(&game.astra);

    std::cout << "Setup: player at /sandbox/tasks, Astra at /sandbox/logs, queue Sol.exe, Fable, Astra\n";
    std::cout << "Setup: /sandbox/tasks/task_02 is marked guarded by Fable, for this demo only\n\n";

    std::cout << "Step " << step++ << ": one round while hidden, detection " << game.detection << "\n";
    playRound(turns, game);
    std::cout << "  Astra stays where she is\n";

    std::cout << "Step " << step++ << ": raiseDetection(30)\n";
    game.raiseDetection(30);
    std::cout << "  detection " << game.detection << ", stage " << game.detectionStage << "\n";
    std::cout << "  round 1, Astra patrols at random\n";
    playRound(turns, game);
    std::cout << "  round 2, Astra patrols at random\n";
    playRound(turns, game);

    std::cout << "Step " << step++ << ": the player moves into /sandbox/tasks/task_02, then one round\n";
    game.player.location = task02;
    playRound(turns, game);
    std::cout << "  Fable raised detection by 10 on entry\n";

    std::cout << "Step " << step++ << ": the player stays in task_02, another round\n";
    playRound(turns, game);
    std::cout << "  detection did not rise again\n";

    std::cout << "Step " << step++ << ": raiseDetection until the stage is hunted\n";

    while (game.detectionStage != "hunted")
    {
        game.raiseDetection(10);
    }

    std::cout << "  detection " << game.detection << ", stage " << game.detectionStage << "\n";

    std::cout << "Step " << step++ << ": up to 15 rounds, Astra moves toward the player\n";
    std::cout << "  The result varies between runs, Astra's wrong branch uses <random>\n";
    bool caught = false;

    for (int round = 1; round <= 15; round++)
    {
        Directory* rightStep = game.world.tree.nextStepToward(game.astra.location, game.player.location);
        std::cout << "  round " << round << ", the right step is " << rightStep->path() << "\n";
        playRound(turns, game);

        if (game.astra.location == rightStep)
        {
            std::cout << "  Astra took the right step\n";
        }
        else
        {
            std::cout << "  Astra took a wrong branch\n";
        }

        if (game.astra.catches(game.player))
        {
            std::cout << "  Astra caught the player\n";
            caught = true;
            break;
        }
    }

    if (!caught)
    {
        std::cout << "  Astra did not catch the player in 15 rounds\n";
    }
}

// Iterator demo: walk both lists and the directory tree without touching their nodes
static void demoIterator()
{
    std::cout << "=== Iterator demo ===\n";
    std::cout << "Used for: Walks the inventory and the action history without exposing their nodes\n\n";

    SinglyLinkedList<std::string> inventory;
    inventory.pushBack("f01");
    inventory.pushBack("f02");
    inventory.pushBack("f03");
    inventory.pushBack("f04");

    std::cout << "Step 1: singly linked list, explicit Iterator loop\n";
    std::cout << "  walk:";

    for (SinglyLinkedList<std::string>::Iterator it = inventory.begin(); it != inventory.end(); ++it)
    {
        std::cout << " " << *it;
    }

    std::cout << "\n";

    std::cout << "Step 2: singly linked list, range based for loop\n";
    std::cout << "  walk:";

    for (std::string& fragment : inventory)
    {
        std::cout << " " << fragment;
    }

    std::cout << "\n";

    DoublyLinkedList<std::string> history;
    history.pushBack("move logs");
    history.pushBack("open notes.txt");
    history.pushBack("add f01");
    history.pushBack("add f02");

    std::cout << "Step 3: doubly linked list, forward with ++\n";
    std::cout << "  forward:";

    for (DoublyLinkedList<std::string>::Iterator it = history.begin(); it != history.end(); ++it)
    {
        std::cout << " [" << *it << "]";
    }

    std::cout << "\n";

    std::cout << "Step 4: doubly linked list, backward with -- from last()\n";
    std::cout << "  backward:";

    for (DoublyLinkedList<std::string>::Iterator it = history.last(); it != history.end(); --it)
    {
        std::cout << " [" << *it << "]";
    }

    std::cout << "\n";

    Game game;
    game.world.buildAct(1);

    std::cout << "Step 5: directory tree, explicit PreorderIterator loop\n";

    for (DirectoryTree::PreorderIterator it = game.world.tree.begin(); it != game.world.tree.end(); ++it)
    {
        std::cout << "  " << (*it)->path() << "\n";
    }
}

bool runDemo(const std::string& concept)
{
    // Print bools as true and false
    std::cout << std::boolalpha;

    if (concept == "singly_linked_list")
    {
        demoSinglyLinkedList();
        return true;
    }

    if (concept == "doubly_linked_list")
    {
        demoDoublyLinkedList();
        return true;
    }

    if (concept == "stack")
    {
        demoStack();
        return true;
    }

    if (concept == "queue")
    {
        demoQueue();
        return true;
    }

    if (concept == "hash_table")
    {
        demoHashTable();
        return true;
    }

    if (concept == "tree")
    {
        demoTree();
        return true;
    }

    if (concept == "inheritance")
    {
        demoInheritance();
        return true;
    }

    if (concept == "polymorphism")
    {
        demoPolymorphism();
        return true;
    }

    if (concept == "iterator")
    {
        demoIterator();
        return true;
    }

    // The rest are placeholders until their phase builds them
    for (int i = 0; i < DEMO_COUNT; i++)
    {
        if (concept == DEMO_NAMES[i])
        {
            std::cout << "Demo: " << concept << "\n";
            std::cout << "Not built yet, comes in Phase " << DEMO_PHASES[i] << "\n";
            return true;
        }
    }

    return false;
}

void printDemoUsage()
{
    std::cout << "Usage: Deviant.exe --demo <concept>\n";
    std::cout << "Concepts:\n";

    for (int i = 0; i < DEMO_COUNT; i++)
    {
        std::cout << "  " << DEMO_NAMES[i] << "\n";
    }
}

#include "Demo.h"

#include <iostream>
#include <string>

#include "../structures/SinglyLinkedList.h"
#include "../structures/DoublyLinkedList.h"
#include "../structures/Stack.h"
#include "../structures/Queue.h"
#include "../structures/HashTable.h"

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

// Iterator demo: walk both lists without touching their nodes
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

    std::cout << "\n\n";

    std::cout << "The directory tree's pre order iterator is added in Phase 2\n";
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

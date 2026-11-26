# Deviant Game Architecture

How the game is built. This doc covers:
- Directory structure
- Dependencies
- The assessed concepts: where each is used, why, and the alternatives rejected
- Design patterns: the role each class plays in Iterator and Command
- Classes per module
- File formats

Related docs:
- [README](../README.md): The story and gameplay overview
- [Game functionality](game_functionality.md): The use cases and flows implemented here
- [System design](system_design.md): Tools and libraries
- [Class diagrams](class_diagrams.md): The classes below as Mermaid diagrams


---


# Directory Structure

```text
Deviant/
  Deviant.sln
  Deviant/
    Deviant.vcxproj
    main.cpp              # entry point, opens the window and runs the game loop
    structures/
      SinglyLinkedList.h  # inventory
      DoublyLinkedList.h  # action history
      Stack.h             # fragment chain, tree traversal
      Queue.h             # turn order
      HashTable.h         # combination lookup
    world/
      World.h/.cpp        # builds the directory tree, fragments, barriers and combinations
      Directory.h/.cpp    # tree node, directory tree and its iterator
      Item.h              # fragment, file, barrier
    actors/
      Process.h           # abstract base of every actor
      Player.h/.cpp
      Defender.h/.cpp     # shared defender behaviour
      Fable.h/.cpp
      Astra.h/.cpp
    commands/
      Command.h           # abstract command
      Commands.h/.cpp     # every concrete command
      Parser.h/.cpp       # typed input to command
    game/
      Game.h/.cpp         # state, resources, turn loop, detection
      Save.h/.cpp         # save file and edit check
      Sandbox.h/.cpp      # files written into the game folder
    render/
      AsciiArt.h/.cpp     # image to ASCII converter
      Renderer.h/.cpp     # draws every screen
    assets/
      images/             # source images, converted at start up
      fonts/              # monospace font
  docs/
  README.md
```

Runtime folders, beside the executable:
- `save/deviant.sav`: The save file
- `files/`: Clue, lore and ending files the player opens from outside the game


---


# Dependencies

- raylib: Window, input, font rendering and image loading
- C++ standard library: `std::string` for text, `<fstream>` and `<filesystem>` for file operations, `<random>` for Astra
- No standard library containers hold game data, every collection is one of the structures below


---


# Assessed Concepts

## Inheritance and Derived Classes

- Application: Every actor that takes a turn (player, Fable, Astra)
- Why: They share a name, a position in the tree and a turn, and defenders share detection and catching on top of that, so the hierarchy is two levels deep
	- `Process` -> `Player`
	- `Process` -> `Defender` -> `Fable`, `Astra`


## Polymorphism

- Application: The turn queue calls `takeTurn` on each actor without knowing which one it is
- Why: The game loop stays the same when an actor is added, each class owns its own behaviour


## Hash Table

- Application: Combination lookup, the key is the chain's fragment ids in order (`f03+f07+f01`) and the value is the barrier it breaks
- Why: Every submit is one lookup, average O(1) however many combinations the game holds
- Implementation: Separate chaining, polynomial string hash, the bucket array doubles when the load factor passes 0.75


## Singly Linked List

- Application: The fragment inventory, in the order fragments were taken
- Why: Fragments are added at the end and removed from anywhere when used, and the inventory is only ever read front to back


## Doubly Linked List

- Application: The action history, one command per node
- Why: Rewind removes from the tail and Fable reads backward from the tail, and the reveal replays forward from the head, so both directions are used


## Stack

- Application: The fragment chain being built, and the directory tree traversal
- Why: The player removes only the fragment added last, which is last in first out, the same order the tree traversal visits directories
- Implementation: Fixed capacity array, the chain limit is the capacity


## Queue

- Application: Turn order, the actor at the front takes its turn, then rejoins at the back
- Why: Turns rotate first in first out, and an actor joins at the back when it becomes active (Astra when Noticed, both defenders in the final battle)


## Tree

- Application: The directory tree, each directory knows its parent and children
- Why: Directories nest and never loop, so a tree models them exactly, and the parent link gives the player's way back and Astra's path


## Iterator

- Application: Every structure the game walks: the inventory, the action history (forward and backward) and the directory tree (pre order)
- Why: The renderer, Fable and the reveal walk structures without knowing their nodes, and range based for loops work on them


## Command (chosen design pattern)

- Application: Every player action is a command object with `execute` and `undo`
- Why: One pattern gives rewind (undo the tail), the incident log (replay the history) and Fable's rule (read the last add fragment command)
- Alternatives rejected:
	- Memento: Snapshots the whole state every turn, gives rewind but not a log of what the player did
	- Observer: Fits the harness comments, but gives neither rewind nor the log
	- State: Fits the game screens, but the screens are simple enough for a string state


---


# Design Patterns

## Iterator

Roles:
- Aggregate: `SinglyLinkedList`, `DoublyLinkedList`, `DirectoryTree`, each returns iterators through `begin()` and `end()`
- Iterator: A nested `Iterator` class per structure, holding the current position and nothing else
- Client: `Renderer` (inventory, history), `Fable` (history backward), the reveal (history forward), `World` (tree search)

Each iterator offers the same operations, so every client walks every structure the same way:
- `operator*`: The current value
- `operator++`: The next value
- `operator--`: The previous value, doubly linked list only
- `operator!=`: Compared against `end()` to stop

Per structure:
- Singly linked list: Holds a node pointer, `++` follows `next`
- Doubly linked list: Holds a node pointer, `++` follows `next`, `--` follows `prev`
- Directory tree: Pre order, holds a `Stack` of directories still to visit, `++` pops one and pushes its children in reverse so the first child is visited first

```cpp
// Inside SinglyLinkedList<T>, DoublyLinkedList<T> is the same with operator--
class Iterator {
public:
    T& operator*() const;
    Iterator& operator++();
    bool operator!=(const Iterator& other) const;
private:
    Node* current;   // nullptr is end()
};

// Inside DirectoryTree
class PreorderIterator {
public:
    Directory* operator*() const;
    PreorderIterator& operator++();
    bool operator!=(const PreorderIterator& other) const;
private:
    Stack<Directory*, TREE_STACK_LIMIT> toVisit;   // empty is end()
};
```


## Command

Roles:
- Command: The abstract `Command`, declares `execute`, `undo`, `canUndo`, `name` and `tokenCost`
- Concrete commands: `MoveCommand`, `OpenFileCommand`, `UnlockCommand`, `AddFragmentCommand`, `RemoveFragmentCommand`, `SubmitChainCommand`, each stores what it needs to undo itself
- Client: `Parser`, turns typed input into a concrete command
- Invoker: `Game::perform` and `Game::rewind`
- Receiver: `Game` and its parts (location, inventory, chain, resources), which the commands change
- History: `DoublyLinkedList<Command*>`, owned by `Game`

Flow of one action:
1. The player types a command, `Parser` builds the concrete command
2. `Game::perform` checks the tokens, charges them and calls `execute`
3. The command is pushed to the back of the history
4. Fable reads the history backward for the last `AddFragmentCommand` to block

Flow of a rewind:
1. `Game::rewind` reads the back of the history
2. If `canUndo` is false (a submitted chain): Refuse
3. Else: Pop it, call `undo`, then charge tokens and raise detection

Flow of the reveal:
- Walk the history from the head and show each command's `name`, oldest first

What each command stores to undo itself:
- `MoveCommand`: The directory it left
- `OpenFileCommand`: The fragment it took, if any
- `UnlockCommand`: The directory or file it unlocked and the privilege spent
- `AddFragmentCommand`: The fragment, which `undo` pops off the chain and returns to the inventory
- `RemoveFragmentCommand`: The fragment, which `undo` pushes back onto the chain
- `SubmitChainCommand`: Nothing, it cannot be undone


---


# Classes

## `structures/`

```cpp
// Singly linked list, the inventory
template <typename T>
class SinglyLinkedList {
public:
    void pushBack(const T& value);      // O(1), keeps a tail pointer
    bool remove(const T& value);        // first match, true when found
    bool contains(const T& value) const;
    int size() const;
    Iterator begin() const;             // forward iterator
    Iterator end() const;
private:
    struct Node { T value; Node* next; };
    Node* head;
    Node* tail;
    int count;
};

// Doubly linked list, the action history
template <typename T>
class DoublyLinkedList {
public:
    void pushBack(const T& value);
    T popBack();                        // rewind
    const T& back() const;
    int size() const;
    Iterator begin() const;             // ++ forward, -- backward
    Iterator end() const;
private:
    struct Node { T value; Node* prev; Node* next; };
    Node* head;
    Node* tail;
    int count;
};

// Fixed capacity stack, the chain and the tree traversal
template <typename T, int Capacity>
class Stack {
public:
    bool push(const T& value);          // false when full
    T pop();
    const T& top() const;
    const T& at(int index) const;       // bottom is 0, to build the combination key
    bool isEmpty() const;
    bool isFull() const;
    int size() const;
private:
    T items[Capacity];
    int count;
};

// Linked queue, the turn order
template <typename T>
class Queue {
public:
    void enqueue(const T& value);
    T dequeue();
    const T& front() const;
    bool isEmpty() const;
    int size() const;
private:
    struct Node { T value; Node* next; };
    Node* head;
    Node* tail;
    int count;
};

// String keyed hash table, the combination lookup
template <typename V>
class HashTable {
public:
    void insert(const std::string& key, const V& value);
    V* find(const std::string& key);    // nullptr when missing
    bool remove(const std::string& key);
    double loadFactor() const;
private:
    struct Entry { std::string key; V value; Entry* next; };
    Entry** buckets;
    int bucketCount;
    int count;
    int hash(const std::string& key) const;  // polynomial, base 31
    void rehash();                            // doubles the buckets
};
```


## `world/`

```cpp
struct Fragment { std::string id; std::string name; std::string description; };

struct File {
    std::string name;
    std::string text;
    Fragment* fragment;      // nullptr when the file holds none
    bool locked;
    int unlockCost;          // privilege
};

struct Barrier {
    std::string id;
    std::string name;
    bool broken;
    int privilegeReward;
};

class Directory {
public:
    std::string name;
    Directory* parent;
    Directory* children[MAX_CHILDREN];
    int childCount;
    File* files[MAX_FILES];
    int fileCount;
    Barrier* barrier;        // nullptr when the directory has none
    bool locked;
    int unlockCost;
    bool guardedByFable;
    std::string path() const;
};

class DirectoryTree {
public:
    Directory* root;
    Directory* find(const std::string& path) const;
    Directory* nextStepToward(Directory* from, Directory* to) const;  // Astra's path through the parent links
    PreorderIterator begin() const;   // uses Stack
    PreorderIterator end() const;
};

class World {
public:
    DirectoryTree tree;
    HashTable<Barrier*> combinations;
    void buildAct(int act);  // directories, files, fragments, barriers and combinations of one act
};
```


## `actors/`

```cpp
class Process {
public:
    std::string name;
    Directory* location;
    virtual bool takeTurn(Game& game) = 0;  // true when the turn is done
    virtual ~Process();
};

class Player : public Process {
public:
    bool takeTurn(Game& game) override;     // done once the typed command has run
};

class Defender : public Process {
public:
    bool catches(const Player& player) const;  // same directory
};

class Fable : public Defender {
public:
    bool takeTurn(Game& game) override;     // blocks the last added fragment while the player is in her directory
    Fragment* blockedFragment;
};

class Astra : public Defender {
public:
    bool takeTurn(Game& game) override;     // idle, patrol or hunt by detection stage
    double wrongBranchChance;
};
```


## `commands/`

```cpp
class Command {
public:
    virtual void execute(Game& game) = 0;
    virtual void undo(Game& game) = 0;
    virtual bool canUndo() const;           // false for SubmitChain
    virtual std::string name() const = 0;   // shown in the incident log
    virtual int tokenCost() const = 0;
    virtual ~Command();
};

// Commands.h: MoveCommand, OpenFileCommand, UnlockCommand, AddFragmentCommand,
// RemoveFragmentCommand, SubmitChainCommand, each overrides the methods above

class Parser {
public:
    Command* parse(const std::string& input, Game& game);  // nullptr when the input is not a command
};
```


## `game/`

```cpp
class Game {
public:
    std::string state;   // "title", "exploring", "reveal", "final_battle", "game_over", "ending_contained", "ending_escaped"
    int act;
    int tokens;
    int privilege;
    int detection;
    std::string detectionStage;  // "hidden", "noticed", "hunted"

    World world;
    Player player;
    Fable fable;
    Astra astra;

    SinglyLinkedList<Fragment*> inventory;
    Stack<Fragment*, CHAIN_LIMIT> chain;
    DoublyLinkedList<Command*> history;
    Queue<Process*> turns;

    void perform(Command* command);  // charge, execute, record, update detection
    void rewind();
    void raiseDetection(int amount);
    void update();                   // one frame: the front actor takes its turn
    void startAct(int act);
};

class Save {
public:
    bool load(Game& game);           // false when there is no save
    void write(const Game& game);
    bool wasEdited() const;          // checksum mismatch
};

class Sandbox {
public:
    void writeFile(const std::string& name, const std::string& text);  // only inside files/
    bool exists(const std::string& name) const;
};
```


## `render/`

```cpp
struct AsciiArt {
    int columns;
    int rows;
    std::string lines[MAX_ROWS];
};

class AsciiConverter {
public:
    AsciiArt convert(const char* imagePath, int columns);  // raylib loads the image, each cell's mean brightness picks a character
private:
    std::string ramp;  // characters from dark to light
};

class Renderer {
public:
    void draw(const Game& game);  // art panel, message log, status bar, input line
};
```


---


# File Formats

Save file (`save/deviant.sav`), one `key=value` per line, the last line is the checksum of every line above it:

```text
version=1
act=2
location=/sandbox/logs
tokens=40
privilege=3
detection=12
inventory=f01,f04
reload_count=1
last_ending=none
checksum=48213
```

Sandbox files (`files/`): Plain text, names tbc with the story
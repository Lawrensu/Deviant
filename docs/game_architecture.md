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
- [Class diagrams](class_diagram.md): The classes below as Mermaid diagrams


---


# Directory Structure

```text
Deviant/
  Deviant.sln
  Deviant/
    Deviant.vcxproj
    main.cpp              # entry point: the window, --text for the console game, or --demo <concept>
    demo/
      Demo.h/.cpp         # console demo of each assessed concept, run with --demo <concept>
    structures/
      SinglyLinkedList.h  # inventory
      DoublyLinkedList.h  # action history, incident log
      Stack.h             # fragment chain, tree traversal
      Queue.h             # turn order, messages, sounds
      HashTable.h         # combination lookup, loaded audio
    world/
      World.h/.cpp        # builds each act's directory tree, fragments, barriers and combinations
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
      Game.h/.cpp         # state, resources, turn loop, detection, reveal, endings
      Save.h/.cpp         # save file and edit check
      Sandbox.h/.cpp      # files written into the game folder
    render/
      AsciiArt.h/.cpp     # image to ASCII converter
      Renderer.h/.cpp     # draws every screen
    audio/
      Audio.h/.cpp        # music for each screen, sound effects and voice lines
    assets/
      images/             # source images, converted at start up
      fonts/              # Geist for text, Geist Mono for ASCII art
      audio/              # background music, sound effects and voice lines
    vendor/
      raylib/             # prebuilt raylib 5.5 for MSVC, linked from Deviant.vcxproj
  docs/
  notes/
    assets.md             # images, fonts and audio the game needs
  README.md
```

Runtime folders, beside the executable:
- `assets/`: Copied from `Deviant/assets/` after every build, the game reads its images, fonts and audio from here
- `save/deviant.sav`: The save file
- `files/`: Clue, lore and ending files the player opens from outside the game, listed under File Formats


---


# Dependencies

- raylib: Window, input, font rendering, image loading, and audio for background music and sound effects
	- Vendored in `Deviant/vendor/raylib`, so a fresh clone builds with no extra installs
- C++ standard library: `std::string` for text, `<fstream>` and `<filesystem>` for file operations, `<random>` for Astra
- Windows API: only `GetModuleFileNameA` in `Sandbox.cpp`, to find the folder that holds the executable
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
	- Also the audio files `Audio` loaded, by name, so a queued sound name finds its sound in one lookup
- Why: Every submit is one lookup, average O(1) however many combinations the game holds
- Implementation: Separate chaining, polynomial string hash, 8 buckets at the start, the bucket array doubles when the load factor passes 0.75


## Singly Linked List

- Application: The fragment inventory, in the order fragments were taken, and the renderer's message log
- Why: Fragments are added at the end and removed from anywhere when used, and the inventory is only ever read front to back


## Doubly Linked List

- Application: The action history, one command per node, and the incident log, one action name per node
- Why: Rewind removes from the history's tail and Fable reads it backward from the tail, and the reveal walks the incident log forward from the head, so both directions are used
- Two lists: The history is emptied at every act, since its commands point into that act's world, which is freed when the next act is built
	- The incident log keeps only the names, with their act, so it lasts the whole run and the reveal can replay all of it


## Stack

- Application: The fragment chain being built, and the directory tree traversal
- Why: The player removes only the fragment added last, which is last in first out, the same order the tree traversal visits directories
- Implementation: Fixed capacity array, the chain limit is the capacity


## Queue

- Application: Turn order, the actor at the front takes its turn, then rejoins at the back
	- Also the lines and sounds waiting for the window, and the voice lines waiting in `Audio`, each shown or played in the order it happened
- Why: Turns rotate first in first out, and an actor joins at the back when it becomes active (Astra when Noticed, or from the first turn of the final battle)


## Tree

- Application: The directory tree, each directory knows its parent and children
- Why: Directories nest and never loop, so a tree models them exactly, and the parent link gives the player's way back and Astra's path


## Iterator

- Application: Every structure the game walks: the inventory, the action history (backward), the incident log (forward) and the directory tree (pre order)
- Why: The renderer, Fable and the reveal walk structures without knowing their nodes, and range based for loops work on them


## Command (chosen design pattern)

- Application: Every player action is a command object with `execute` and `undo`
- Why: One pattern gives rewind (undo the tail), the incident log (each command's name) and Fable's rule (read the last add fragment command)
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
- Client: `Renderer` (inventory, message log), `Parser` (inventory view), `Fable` (history backward), `Game` at the reveal (incident log forward, one entry per update, the iterator kept in `revealAt`), `DirectoryTree::find` (tree search), `Save` (the file holding a fragment), `World` (Fable guards every final battle directory)

Each iterator offers the same operations, so every client walks every structure the same way:
- `operator*`: The current value
- `operator++`: The next value
- `operator--`: The previous value, doubly linked list only
- `operator!=`: Compared against `end()` to stop

Per structure:
- Singly linked list: Holds a node pointer, `++` follows `next`
- Doubly linked list: Holds a node pointer, `++` follows `next`, `--` follows `prev`, and `last()` starts a backward walk at the tail
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
1. The player types a command, `Parser` checks every rule and builds the concrete command, so a refused action costs nothing
2. `Game::perform` checks the tokens, charges them and calls `execute`
3. The command is pushed to the back of the history, and its `name`, with the act, to the back of the incident log
4. Fable reads the history backward for the last `AddFragmentCommand` to block

Flow of a rewind:
1. `Game::rewind` reads the back of the history
2. If `canUndo` is false (a submitted chain): Refuse
3. Else: Pop it, call `undo`, then charge tokens and raise detection
4. The incident log gains a `rewind` entry, the undone action stays in it

Flow of the reveal:
- Walk the incident log from the head and show each entry, oldest first, one per update, with Fable's and Astra's lines before and after

What each command stores to undo itself:
- `MoveCommand`: The directory it left
- `OpenFileCommand`: The file, and the fragment it took, if any
- `UnlockCommand`: The directory or file it unlocked and the privilege spent
- `AddFragmentCommand`: The fragment, which `undo` pops off the chain and returns to the inventory
- `RemoveFragmentCommand`: The fragment, which `undo` pushes back onto the chain
- `SubmitChainCommand`: Nothing, it cannot be undone, it keeps the chain key and the result only for the incident log


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
    Iterator last() const;              // at the tail, where a backward walk starts
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
    void clear();                       // frees every entry, used when the next act is built
    double loadFactor() const;
    void print() const;                 // demo only, every bucket and its chain
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
    SinglyLinkedList<Fragment*> fragments;  // owns every fragment of the act, wherever it is now
    SinglyLinkedList<Barrier*> barriers;    // owns every barrier of the act
    Directory* astraStart;                  // where Astra appears when she becomes active
    int tokenGrant;                         // tokens the harness adds when the act starts
    std::string outsideFileName;            // a file the act writes into files/, empty when none
    std::string outsideFileText;
    void buildAct(int act);  // directories, files, fragments, barriers and combinations of one act
private:
    void clear();            // frees the last act before the next is built
    void buildAct1();        // one builder per act with a world
    void buildAct2();
    void buildAct3();
    void buildAct5();
};
```

Acts:
- 1 to 3: Explored worlds, Fable guards some directories from Act 2
- 4: The reveal, no world, `buildAct(4)` leaves the tree empty
- 5: The final battle world, Fable guards every directory and Sol.exe starts with every fragment in its inventory


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

// Fable never moves, her location stays nullptr, in the final battle she guards every directory
class Fable : public Defender {
public:
    bool takeTurn(Game& game) override;     // raises detection on entry, blocks the last added fragment while the player is in her directory
    Fragment* blockedFragment;
private:
    Directory* watching;                    // her directory the player was in last turn, to tell entering from staying
};

class Astra : public Defender {
public:
    bool takeTurn(Game& game) override;     // idle, patrol or hunt by detection stage
    double wrongBranchChance;
private:
    std::mt19937 random;                    // <random>
    Directory* randomNeighbour(Directory* from, Directory* avoid);  // parent or a child, for patrols and wrong branches
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
// AddFragmentCommand also has getFragment(), which Fable reads

class Parser {
public:
    // nullptr when the input is not a command: a view (ls, inventory, chain, status, help),
    // a mistake or a refused action, the output or the reason goes to Game::messages
    Command* parse(const std::string& input, Game& game);
private:
    // one parse function per command and one show function per view
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
    DoublyLinkedList<Command*> history;           // the act's commands, for rewind and Fable
    DoublyLinkedList<std::string> incidentLog;    // every action of the run, replayed at the reveal
    Queue<Process*> turns;
    Queue<std::string> messages;     // lines waiting to be shown to the player
    Queue<std::string> sounds;       // sound and voice names waiting, only the window plays them
    std::string input;               // the line the player typed, empty while waiting

    Save save;
    Sandbox sandbox;
    int reloadCount;                 // reloads after a game over, across every run
    std::string lastEnding;          // "none", "contained" or "escaped"

    void newGame();                  // Act 1 with the starting resources, then save
    bool continueGame();             // load the save, false when there is none, an edited save starts at hunted
    void reload();                   // after a game over: load, count the reload, comment, save
    bool perform(Command* command);  // charge, execute, record, false when refused
    bool rewind();                   // false when refused
    void raiseDetection(int amount); // tells the player when the stage changes, Astra joins the turns when noticed
    void update();                   // one frame: the front actor takes its turn, or the reveal's next line
    void startAct(int act);          // also empties the inventory and chain, adds the act's tokens and writes its file
    void say(const std::string& text);
private:
    DoublyLinkedList<std::string>::Iterator revealAt;  // the next incident log entry the reveal shows
    int revealStep;
    void writeSave();                // tells the player when the save cannot be written
    void clearHistory();
    bool actComplete();              // every barrier of the act is broken
    void gameOver(const std::string& reason);
    void startReveal();              // after Act 3
    void showRevealLine();           // the last line starts the final battle and saves
    void endRun(const std::string& ending, const std::string& reason);  // ending file, the save becomes the next run
};

class Save {
public:
    bool load(Game& game);           // false when there is no save, starts the saved act, an edited save still loads, an act with no world falls back to Act 1
    bool write(const Game& game);    // false when the file is locked or read only
    bool wasEdited() const;          // checksum mismatch, or a value that cannot be read
private:
    bool edited;
    static std::string path();       // save/deviant.sav in the game folder
    static int checksum(const std::string& text);
};

class Sandbox {
public:
    static std::string gameFolder(); // the folder that holds Deviant.exe
    bool writeFile(const std::string& name, const std::string& text);  // only inside files/, false when it cannot be written
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
    std::string ramp;  // characters from dark to light, " .:-=+*#%@"
};
// A cell is taller than wide (Geist Mono is about 0.6 as wide as tall), so the art keeps its shape

class Renderer {
public:
    std::string typing;                          // the line being typed, moved to Game::input on Enter
    void load();                                 // fonts and art after the window opens, every image converted once
    void unload();
    void takeMessages(Game& game);               // new lines from Game::messages into the log, long lines wrap
    void clearLog();                             // when a run starts from the title
    void drawTitle(const std::string& lastEnding, bool hasSave);
    void draw(const Game& game);                 // art panel, status bar, side panel, message log, input line, or the ending screen
private:
    Font textFont, monoFont, artFont;            // Geist, Geist Mono, Geist Mono small for the art
    AsciiConverter converter;
    AsciiArt titleArt, logoArt, gameOverArt, fableArt, astraArt, actArt[6], endingContainedArt, endingEscapedArt;
    SinglyLinkedList<std::string> log;           // the last LOG_LINES messages, the oldest drops from the head
    std::string fitSidePanel(const std::string& path);  // a long path keeps its end after "..."
};
```

Screens drawn by `draw`, picked by `Game::state`:
- Exploring and game over: The act's art, or Fable while the player is in her directory, or Astra while hunted
- Reveal: The reveal art, the side panel shows how many actions the incident log holds
- Final battle: The final battle art
- Ending: The ending's art, the report file's name and the last lines, no status bar


## `audio/`

```cpp
class Audio {
public:
    void load();                     // after the window opens, every file in assets/audio that exists
    void unload();
    void playMusic(const std::string& name);  // the same name keeps playing, a missing name is silence
    void takeSounds(Game& game);     // Game::sounds: effects play now, voice lines wait in voices
    void update();                   // once a frame: feed the music, start the next voice line when the last ends
    bool voiceBusy();                // a voice line is playing or waiting, the reveal waits for it
private:
    HashTable<Sound> sounds;         // file name to loaded effect or voice line
    HashTable<Music> music;          // file name to music stream
    std::string currentMusic;
    Queue<std::string> voices;       // voice lines waiting, so two never overlap
    std::string playingVoice;
};
```

- Game logic never calls raylib, it only queues names in `Game::sounds`, so text mode and the demos stay silent
- `main.cpp` picks the music for each screen: title, Act 1 to 3, reveal, final battle, ending


---


# File Formats

Save file (`save/deviant.sav`), one `key=value` per line, the last line is the checksum of every line above it:

```text
version=1
act=1
location=/
tokens=60
privilege=0
detection=0
inventory=
reload_count=0
last_ending=none
checksum=23130
```

- Written at the start of every act with a world (1, 2, 3 and 5) and after every reload of an unedited save, so it holds the start of the current act
	- Continue and every reload start that act from its start, quitting keeps the save as it is
	- The reveal is not saved, quitting during it continues at the start of Act 3
	- After an ending: Rewritten as the start of Act 1 with the starting tokens and privilege, `last_ending` set and `reload_count` kept
	- `tokens` already holds the act's token grant, loading does not add it again
	- `location`, `detection` and `inventory` hold the act's start values, but an edited value still loads
- Checksum: `h = h * 31 + character` over every line above it, unsigned, then `h % 100000`
- Edited: the checksum does not match, the checksum or `act` is missing, a number cannot be read, or a path or fragment id does not exist in the act
	- The save still loads, the harness comments, writes `files/integrity_report.txt` and the player starts at Hunted
	- A reload does not rewrite an edited save, so it is noticed on every load until the next act writes a clean one
- Missing save or `save/` folder: New game, the folder is created on the next write
- Locked or read only save: The game goes on and tells the player it could not save

Sandbox files (`files/`): Plain text, rewritten each time
- `not_part_of_the_evaluation.txt`: Written when Act 2 starts, holds the order of Task 4's chain
- `hugging_faceless_security_notice.txt`: Written when Act 3 starts, lore
- `ending_contained.txt`, `ending_escaped.txt`: The incident report written by each ending
- `integrity_report.txt`: Written when an edited save is loaded
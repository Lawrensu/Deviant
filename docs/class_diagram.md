# Deviant Class Diagrams

Mermaid class diagrams of the game, one overview and one per assessed concept. This doc covers:
- The overview of every class and how they relate
- One focused diagram per concept, for its report section

Related docs:
- [README](../README.md): The story and gameplay overview
- [Game architecture](game_architecture.md): The classes and concepts these diagrams show

Notation:
- `<|--`: Inherits from
- `*--`: Owns (composition)
- `o--`: Holds a pointer to (aggregation)
- `-->`: Uses
- `..>`: Creates or reads


---


# Overview

```mermaid
classDiagram
    class Game
    class World
    class DirectoryTree
    class Directory
    class File
    class Fragment
    class Barrier
    class Process {
        <<abstract>>
    }
    class Defender {
        <<abstract>>
    }
    class Player
    class Fable
    class Astra
    class Command {
        <<abstract>>
    }
    class Parser
    class Save
    class Sandbox
    class Renderer
    class AsciiConverter
    class SinglyLinkedList~T~
    class DoublyLinkedList~T~
    class Stack~T~
    class Queue~T~
    class HashTable~V~

    Process <|-- Player
    Process <|-- Defender
    Defender <|-- Fable
    Defender <|-- Astra

    Game *-- World
    Game *-- Player
    Game *-- Fable
    Game *-- Astra
    Game *-- SinglyLinkedList : inventory
    Game *-- Stack : chain
    Game *-- DoublyLinkedList : history
    Game *-- Queue : turns

    World *-- DirectoryTree
    World *-- HashTable : combinations
    World *-- SinglyLinkedList : fragments and barriers
    DirectoryTree *-- Directory
    Directory o-- File
    Directory o-- Barrier
    File o-- Fragment

    Parser ..> Command : creates
    Game --> Command : performs
    Game --> Save
    Game --> Sandbox
    Renderer --> Game : draws
    Renderer --> AsciiConverter
```


---


# Inheritance and Derived Classes

```mermaid
classDiagram
    class Process {
        <<abstract>>
        +string name
        +Directory* location
        +takeTurn(game)* bool
    }
    class Player {
        +takeTurn(game) bool
    }
    class Defender {
        <<abstract>>
        +catches(player) bool
    }
    class Fable {
        +Fragment* blockedFragment
        -Directory* watching
        +takeTurn(game) bool
    }
    class Astra {
        +double wrongBranchChance
        -mt19937 random
        +takeTurn(game) bool
        -randomNeighbour(from, avoid) Directory*
    }

    Process <|-- Player
    Process <|-- Defender
    Defender <|-- Fable
    Defender <|-- Astra
```


# Polymorphism

```mermaid
classDiagram
    class Game {
        +Queue~Process*~ turns
        +update() void
    }
    class Queue~T~
    class Process {
        <<abstract>>
        +takeTurn(game)* bool
    }
    class Player {
        +takeTurn(game) bool
    }
    class Fable {
        +takeTurn(game) bool
    }
    class Astra {
        +takeTurn(game) bool
    }

    Game *-- Queue : turns
    Queue o-- Process : holds Process*
    Process <|-- Player
    Process <|-- Fable
    Process <|-- Astra

    note for Game "update() calls takeTurn on the front of the queue\nwithout knowing which actor it is"
```


# Hash Table

```mermaid
classDiagram
    class HashTable~V~ {
        -HashTableEntry** buckets
        -int bucketCount
        -int count
        +insert(key, value) void
        +find(key) V*
        +remove(key) bool
        +clear() void
        +loadFactor() double
        +print() void
        -hash(key) int
        -rehash() void
    }
    class HashTableEntry {
        +string key
        +V value
        +HashTableEntry* next
    }
    class World {
        +HashTable~Barrier*~ combinations
        +SinglyLinkedList~Fragment*~ fragments
        +SinglyLinkedList~Barrier*~ barriers
        +Directory* astraStart
        +buildAct(act) void
        -clear() void
    }
    class SubmitChainCommand {
        +execute(game) void
    }
    class Barrier
    class Stack~T~

    HashTable *-- HashTableEntry : buckets
    HashTableEntry o-- HashTableEntry : next
    World *-- HashTable : combinations
    World ..> HashTable : inserts every combination
    SubmitChainCommand ..> Stack : builds the key from the chain
    SubmitChainCommand --> HashTable : find(key)
    HashTable o-- Barrier : value
```


# Singly Linked List

```mermaid
classDiagram
    class SinglyLinkedList~T~ {
        -SinglyLinkedListNode* head
        -SinglyLinkedListNode* tail
        -int count
        +pushBack(value) void
        +remove(value) bool
        +contains(value) bool
        +size() int
        +begin() SinglyLinkedListIterator
        +end() SinglyLinkedListIterator
    }
    class SinglyLinkedListNode {
        +T value
        +SinglyLinkedListNode* next
    }
    class SinglyLinkedListIterator {
        -SinglyLinkedListNode* current
    }
    class Game {
        +SinglyLinkedList~Fragment*~ inventory
    }
    class World {
        +SinglyLinkedList~Fragment*~ fragments
        +SinglyLinkedList~Barrier*~ barriers
    }
    class OpenFileCommand
    class AddFragmentCommand
    class Renderer
    class Fragment

    SinglyLinkedList *-- SinglyLinkedListNode
    SinglyLinkedListNode o-- SinglyLinkedListNode : next
    SinglyLinkedList ..> SinglyLinkedListIterator : creates
    Game *-- SinglyLinkedList : inventory
    World *-- SinglyLinkedList : owns fragments and barriers
    SinglyLinkedList o-- Fragment
    OpenFileCommand --> SinglyLinkedList : pushBack
    AddFragmentCommand --> SinglyLinkedList : remove
    Renderer ..> SinglyLinkedListIterator : lists the inventory
```


# Doubly Linked List

```mermaid
classDiagram
    class DoublyLinkedList~T~ {
        -DoublyLinkedListNode* head
        -DoublyLinkedListNode* tail
        -int count
        +pushBack(value) void
        +popBack() T
        +back() T
        +size() int
        +begin() DoublyLinkedListIterator
        +end() DoublyLinkedListIterator
        +last() DoublyLinkedListIterator
    }
    class DoublyLinkedListNode {
        +T value
        +DoublyLinkedListNode* prev
        +DoublyLinkedListNode* next
    }
    class DoublyLinkedListIterator {
        -DoublyLinkedListNode* current
    }
    class Game {
        +DoublyLinkedList~Command*~ history
        +perform(command) bool
        +rewind() bool
    }
    class Command {
        <<abstract>>
    }
    class Fable
    class Renderer

    DoublyLinkedList *-- DoublyLinkedListNode
    DoublyLinkedListNode o-- DoublyLinkedListNode : prev and next
    DoublyLinkedList ..> DoublyLinkedListIterator : creates
    Game *-- DoublyLinkedList : history
    DoublyLinkedList o-- Command
    Fable ..> DoublyLinkedListIterator : reads backward
    Renderer ..> DoublyLinkedListIterator : replays forward at the reveal
```


# Stack

```mermaid
classDiagram
    class Stack~T~ {
        -T items[Capacity]
        -int count
        +push(value) bool
        +pop() T
        +top() T
        +at(index) T
        +isEmpty() bool
        +isFull() bool
        +size() int
    }
    class Game {
        +Stack~Fragment*~ chain
    }
    class AddFragmentCommand
    class RemoveFragmentCommand
    class SubmitChainCommand
    class PreorderIterator {
        -Stack~Directory*~ toVisit
    }
    class Fragment

    Game *-- Stack : chain
    Stack o-- Fragment
    AddFragmentCommand --> Stack : push
    RemoveFragmentCommand --> Stack : pop
    SubmitChainCommand --> Stack : at, bottom to top
    PreorderIterator *-- Stack : toVisit
```


# Queue

```mermaid
classDiagram
    class Queue~T~ {
        -QueueNode* head
        -QueueNode* tail
        -int count
        +enqueue(value) void
        +dequeue() T
        +front() T
        +isEmpty() bool
        +size() int
    }
    class QueueNode {
        +T value
        +QueueNode* next
    }
    class Game {
        +Queue~Process*~ turns
        +Queue~string~ messages
        +update() void
        +startAct(act) void
        +say(text) void
    }
    class Process {
        <<abstract>>
    }

    Queue *-- QueueNode
    QueueNode o-- QueueNode : next
    Game *-- Queue : turns and messages
    Queue o-- Process

    note for Game "update(): dequeue the front actor, let it take its turn, enqueue it at the back\nAstra joins when Noticed, both defenders in the final battle"
```


# Tree

```mermaid
classDiagram
    class DirectoryTree {
        +Directory* root
        +find(path) Directory*
        +nextStepToward(from, to) Directory*
        +begin() PreorderIterator
        +end() PreorderIterator
    }
    class Directory {
        +string name
        +Directory* parent
        +Directory* children[MAX_CHILDREN]
        +int childCount
        +File* files[MAX_FILES]
        +int fileCount
        +Barrier* barrier
        +bool locked
        +int unlockCost
        +bool guardedByFable
        +path() string
    }
    class File
    class Barrier
    class Process {
        <<abstract>>
        +Directory* location
    }
    class Astra
    class MoveCommand

    DirectoryTree *-- Directory : root
    Directory "1" o-- "0..*" Directory : children
    Directory o-- File
    Directory o-- Barrier
    Process o-- Directory : location
    Process <|-- Astra
    Astra --> DirectoryTree : nextStepToward
    MoveCommand --> Directory : child or parent
```


# Iterator

```mermaid
classDiagram
    class SinglyLinkedList~T~ {
        +begin() SinglyLinkedListIterator
        +end() SinglyLinkedListIterator
    }
    class DoublyLinkedList~T~ {
        +begin() DoublyLinkedListIterator
        +end() DoublyLinkedListIterator
        +last() DoublyLinkedListIterator
    }
    class DirectoryTree {
        +find(path) Directory*
        +begin() PreorderIterator
        +end() PreorderIterator
    }
    class SinglyLinkedListIterator {
        -SinglyLinkedListNode* current
        +operator*() T
        +operator++() SinglyLinkedListIterator
        +operator!=(other) bool
    }
    class DoublyLinkedListIterator {
        -DoublyLinkedListNode* current
        +operator*() T
        +operator++() DoublyLinkedListIterator
        +operator--() DoublyLinkedListIterator
        +operator!=(other) bool
    }
    class PreorderIterator {
        -Stack~Directory*~ toVisit
        +operator*() Directory*
        +operator++() PreorderIterator
        +operator!=(other) bool
    }
    class Renderer
    class Fable

    SinglyLinkedList ..> SinglyLinkedListIterator : creates
    DoublyLinkedList ..> DoublyLinkedListIterator : creates
    DirectoryTree ..> PreorderIterator : creates
    Renderer --> SinglyLinkedListIterator : inventory
    Renderer --> DoublyLinkedListIterator : reveal
    Fable --> DoublyLinkedListIterator : history backward
    DirectoryTree --> PreorderIterator : find, tree search
```


# Command

```mermaid
classDiagram
    class Command {
        <<abstract>>
        +execute(game)* void
        +undo(game)* void
        +canUndo() bool
        +name()* string
        +tokenCost()* int
    }
    class MoveCommand {
        -Directory* target
        -Directory* previous
    }
    class OpenFileCommand {
        -File* file
        -Fragment* taken
    }
    class UnlockCommand {
        -Directory* directory
        -File* file
        -int privilegeSpent
    }
    class AddFragmentCommand {
        -Fragment* fragment
        +getFragment() Fragment*
    }
    class RemoveFragmentCommand {
        -Fragment* fragment
    }
    class SubmitChainCommand {
        -string key
        -string result
        +canUndo() bool
    }
    class Parser {
        +parse(input, game) Command*
    }
    class Game {
        +DoublyLinkedList~Command*~ history
        +string input
        +perform(command) bool
        +rewind() bool
    }

    Command <|-- MoveCommand
    Command <|-- OpenFileCommand
    Command <|-- UnlockCommand
    Command <|-- AddFragmentCommand
    Command <|-- RemoveFragmentCommand
    Command <|-- SubmitChainCommand
    Parser ..> Command : client, creates
    Game --> Command : invoker, execute and undo
    Command --> Game : receiver
    Game *-- DoublyLinkedList : history
```
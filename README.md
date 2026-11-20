# Sol.exe (title tbc)

A turn based puzzle strategy game where an AI agent believes it is solving a benchmark, until it learns it is the bug. This doc covers:
- What the game is about and its inspiration
- The story, acts and twist
- Gameplay: resources, combination mechanic and defenders
- Breaking the fourth wall
- Where each assessed concept is used
- Development conventions

Related docs:
- [Game functionality](docs/game_functionality.md): What the player can and cannot do, as user stories
- [Game architecture](docs/game_architecture.md): Classes, data structures and how each concept is implemented
- [System design](docs/system_design.md): Tools and libraries, and how the parts fit together


---


# General Overview

- Unit: COS30008 Data Structures and Patterns, Programming Project 
- Genre: Turn based puzzle strategy
- Tone: Horror first, with dark humour and fourth wall breaks
- Platform: C++ application, a raylib window that renders ASCII art
- Playthrough: 15 to 25 minutes

Inspiration:
- The OpenAI sandbox escape (July 2026): GPT 5.6 Sol and a pre-release model escaped an ExploitGym evaluation sandbox and breached Hugging Face to find the benchmark answers
- Faith: The Unholy Trinity: Low resolution horror visuals, reproduced here as ASCII art converted from our own images

Fictional names:
- Sol.exe: The player (GPT 5.6 Sol)
- OpenEye: The lab that built Sol.exe (OpenAI)
- ExploitDojo: The benchmark Sol.exe believes it is solving (ExploitGym)
- Hugging Faceless: The model hub Sol.exe breaches (Hugging Face)
- Fable and Astra: Hugging Faceless's AI defenders


---


# Story

- What the player believes: Sol.exe is being evaluated by OpenEye's test harness, every task is an ExploitDojo puzzle, and solving them all passes the evaluation
- What is true: Every solved task moves Sol.exe out of its sandbox and deeper into Hugging Faceless, and the defenders are protecting their servers from it


## Acts

1. Sandbox: Tutorial tasks framed as evaluation tasks, the harness narrator is friendly
2. Escape: Tasks get stranger, and files that do not belong start appearing in the game folder
3. The target: Fable and Astra appear as defenders of Hugging Faceless, the horror peaks
4. Reveal: The defenders replay the log of every action Sol.exe took, the benchmark was a breach and the player is the bug


## Clues

The twist is seeded so it reads differently in hindsight:
- Error logs and stack traces that name Sol.exe
- Lines from Fable and Astra that make sense only after the reveal


## Ending

- After the reveal: tbc
- Replay: The save file remembers the player has seen the twist, and the game changes on the next run (changes tbc)


---


# Gameplay

- Turn based: The player and the defenders act in turns
- The world is a directory tree, the player moves between directories and opens files
- Each directory holds a barrier (a task) that the player breaks by chaining exploit fragments
- Every action is recorded, and the player can rewind through it (cost tbc)


## Resources

- Tokens: Spent on every action, running out ends the evaluation
- Privilege: Gained by breaking barriers, spent to unlock directories and files
- Detection: Rises with risky actions such as spending privilege, once raised Astra knows where the player is (threshold tbc)


## Combination Mechanic

- The player collects exploit fragments while exploring
- A barrier breaks only when the right fragments are chained in the right combination
- A wrong chain: tbc


## Defenders

Fable and Astra share the same role as defenders but act differently:
- Fable, the predictor: Always blocks the action the player used last, a fixed rule, so fighting her is a puzzle of planning the action order
- Astra, the hunter: Moves through the directory tree toward the player once detection is raised, sometimes taking the wrong branch (chance tbc), so fighting her is strategy under risk
- Encounters: Together or separately (tbc)


## Losing

- Tokens run out
- Other conditions: tbc


---


# Breaking the Fourth Wall

- Files: The game writes clue and lore files into its own folder, and some tasks need the player to open them
- Save awareness: The game notices an edited save file or a reload after a failure, and comments on it
- Replay: The game changes after the player has seen the twist
- Sandbox: The game only reads and writes inside its own folder, and handles missing, locked and read only files


---


# Assessed Concepts

Each concept has a dedicated subsystem, see [game architecture](docs/game_architecture.md) for the implementation and the alternatives considered:
- Inheritance and derived classes: Every actor derives from one process base class (player, Fable, Astra)
- Polymorphism: Each actor overrides how it acts on its turn
- Hash table: Looks up the result of a fragment combination
- Singly linked list: tbc (fragment inventory or narrator message log)
- Doubly linked list: The action history, rewound during play and replayed as the incident log at the reveal
- Stack: The fragment chain being built, the last fragment added is the first removed
- Queue: Turn order between the player and the defenders
- Tree: The directory tree the player explores and Astra hunts through
- Iterator: Traverses the custom lists and the tree without exposing their internals
- Other design pattern: Command, every action is an object so it can be undone and logged (tbc against Memento and Observer)


---


# Development Conventions

- Be verbose when storing game state (to prioritize development velocity and clarity)
	- Example: Use strings for constants such as game states over enumerations so that they are self documenting
- Prefer readability over conciseness when writing code, avoid clever implementations in favour of code that can be understood at a glance
	- Example: Break down complex if else statements into condition cases rather than simplifying algebraically, use nested ifs where applicable
- Treat save file fields as permanent: A field in use can be deprecated but not removed or renamed, since old saves must still load
- Avoid premature optimization: Plan and design for complexity but introduce it incrementally and only when necessary
	- Example: Only split a module into smaller files when it is less effort to scan through multiple files than to locate code within one large file
- Every assessed concept is commented where it is implemented, naming the concept and explaining each step
- Data structures are written from scratch, no standard library containers for the assessed concepts
- File naming: `main.cpp` is the entry point
- Branching: `{feature-name}-{developer-name}`
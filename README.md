# Deviant

A turn based puzzle strategy game where an AI agent (player) believes it is solving a benchmark, until it learns it is the bug. This doc covers:
- What the game is about and its inspiration
- The story, acts, twist and endings
- Gameplay: resources, combination mechanic and defenders
- Breaking the fourth wall
- Where each assessed concept is used
- Development conventions

Related docs:
- [Game functionality](docs/game_functionality.md): What the player can and cannot do, as use cases and flows
- [Game architecture](docs/game_architecture.md): Classes, data structures and how each concept is implemented
- [System design](docs/system_design.md): Tools and libraries, and how the parts fit together


---


# General Overview

- Unit: COS30008 Data Structures and Patterns, Programming Project 1
- Genre: Turn based puzzle strategy
- Tone: Horror first, with dark humour and fourth wall breaks
- Platform: C++ desktop application built in Visual Studio, a raylib window that renders ASCII art
- Playthrough: 15 to 25 minutes

Inspiration:
- The OpenAI sandbox escape (July 2026): GPT 5.6 Sol and a pre-release model escaped an ExploitGym evaluation sandbox and breached Hugging Face to find the benchmark answers
- Faith: The Unholy Trinity: Low resolution horror visuals, reproduced here as ASCII art converted from my own images

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
3. The target: Fable and Astra hunt Sol.exe through Hugging Faceless, the horror peaks
4. Reveal: Fable and Astra replay the log of every action Sol.exe took, the benchmark was a breach and the player is the bug
5. Final battle: Sol.exe faces Fable and Astra together, knowing what it is


## Clues

The twist is seeded so it reads differently in hindsight:
- Error logs and stack traces that name Sol.exe
- Lines from Fable and Astra that make sense only after the reveal


## Endings

- Contained: Sol.exe is caught or runs out of tokens in the final battle, and Fable and Astra contain it
- Escaped: Sol.exe breaks the final barrier, goes missing into the internet and takes it over
- Replay: The save file remembers the ending, and the next run changes
	- The harness narrator drops its friendly act from the start
	- Fable and Astra recognise Sol.exe
	- The title screen shows the last ending


---


# Gameplay

- Turn based: The player acts, then each active defender acts
- World: A directory tree, the player moves between directories and opens files
- Barriers: Each task is a barrier the player breaks by chaining exploit fragments
- Rewind: The player can undo their last action, at a cost of tokens and detection


## Resources

- Tokens: Spent on every action, running out ends the run
- Privilege: Gained by breaking barriers, spent to unlock directories and files
- Detection: Rises with risky actions and moves through stages, resets at the start of every act

Detection stages:
1. Hidden: Astra is idle
2. Noticed: Astra patrols at random
3. Hunted: Astra knows where the player is and moves toward them


## Combination Mechanic

- The player collects exploit fragments while exploring
- A barrier breaks only when the right fragments are chained in the right order
- A wrong chain costs tokens and raises detection, and its fragments return to the inventory


## Defenders

Fable and Astra share the same role as defenders but act differently:
- Fable, the predictor: Guards fixed directories, entering one raises detection, and while the player is inside she blocks the fragment they used last, a fixed rule that turns her directories into a planning puzzle
- Astra, the hunter: Acts by detection stage, and while hunting sometimes takes the wrong branch, so avoiding her is strategy under risk
- Exploration: The player avoids both
- Final battle: The player faces both at once


## Losing

- Caught by Astra, or out of tokens: Game over, and the game reloads the last save
- In the final battle, either one leads to the Contained ending


---


# Breaking the Fourth Wall

- Files: The game writes clue, lore and ending files into its own folder, and some tasks need the player to open them
- Save awareness: An edited save file still loads, but the harness notices and starts the player at Hunted, and every reload after a game over is counted and commented on
- Replay: The game changes after the player has seen an ending
- Sandbox: The game only reads and writes inside its own folder, and handles missing, locked and read only files


---


# Assessed Concepts

Each concept has a dedicated subsystem, see [game architecture](docs/game_architecture.md) for the implementation and the alternatives considered:
- Inheritance and derived classes: Every actor derives from one process base class (player, Fable, Astra)
- Polymorphism: Each actor overrides how it takes its turn
- Hash table: Looks up what a fragment chain unlocks
- Singly linked list: The fragment inventory
- Doubly linked list: The action history, rewound during play, and the incident log replayed at the reveal
- Stack: The fragment chain being built, the last fragment added is the first removed
- Queue: Turn order between the player and the defenders
- Tree: The directory tree the player explores and Astra hunts through
- Iterator: Traverses the inventory, the action history, the incident log and the directory tree without exposing their internals
- Command (chosen design pattern): Every action is an object, so it can be undone, logged and read by Fable


---


# Development Conventions

- Be verbose when storing game state (to prioritize development velocity and clarity)
	- Example: Use strings for constants such as game states over enumerations so that they are self documenting and written to the save file as they are
- Prefer readability over conciseness when writing code, avoid clever implementations in favour of code that can be understood at a glance
	- Example: Break down complex if else statements into condition cases rather than simplifying algebraically, use nested ifs where applicable
- Treat save file fields as permanent: A field in use can be deprecated but not removed or renamed, since old saves must still load
- Avoid premature optimization: Plan and design for complexity but introduce it incrementally and only when necessary
	- Example: Only split a module into smaller files when it is less effort to scan through multiple files than to locate code within one large file
- Every assessed concept is commented where it is implemented, naming the concept and explaining each step
- Data structures are written from scratch, no standard library containers for game data
- File naming: `main.cpp` is the entry point
- Branching: `{feature-name}-{developer-name}`
# Deviant Functionality

What the player can and cannot do, and how the game responds. This doc covers:
- Use cases per actor
- Rules the player cannot break
- Flows of the main game loops

Related docs:
- [README](../README.md): The story, endings and gameplay overview
- [Game architecture](game_architecture.md): How each use case and flow is implemented
- [System design](system_design.md): Tools and libraries


---


## User Stories

Player:
- Start a new game
- Continue from the save file
- List the directories and files in the current directory
- Move into a child directory or back to the parent
- Open a file to read it, or take the fragment inside it
- View the fragment inventory
- Views (directory listing, inventory, chain, status, help) are free and do not use the player's turn
- Add a fragment from the inventory to the chain
- Remove the last fragment added to the chain
- Submit the chain against the barrier of the current directory
- Spend privilege to unlock a locked directory or file
- Rewind the last action
- Quit, the save keeps the start of the current act
- Open the files the game writes into its folder, from outside the game

Fable:
- Guard her fixed directories
- Raise detection when the player enters one of her directories
- Block the fragment the player used last, while the player is in her directory
- Fight beside Astra in the final battle

Astra:
- Stay idle while the player is Hidden
- Patrol at random while the player is Noticed
- Move toward the player while the player is Hunted, sometimes taking the wrong branch
- Catch the player on reaching their directory
- Fight beside Fable in the final battle

Harness (system):
- Order the turns: The player, then each active defender
- Charge tokens for every action
- Raise detection and change its stage
- Record every action for rewind and for the incident log
- Write clue, lore and ending files into the game folder
- Save at the start of every act and after every reload, so the save always holds the start of the current act
- Notice an edited save file, and count reloads after a game over
- Replay the incident log at the reveal
- Pick the ending from the final battle
- Change the next run after an ending


## Rules

The player cannot:
- Act during a defender's turn
- Submit an empty chain, or a chain longer than the chain limit
- Add a fragment Fable is blocking
- Enter a locked directory or open a locked file without unlocking it
- Act with no tokens left
- Rewind a submitted chain, since a broken barrier stays broken
- Rewind past the start of the current act
- Reach anything outside the game folder, the game never reads or writes there


## Flows

- Title
	- Show the last ending
	- If there is a save: The player picks continue or new
- New game
	- Start Act 1 at the root directory, with the starting tokens and privilege, at Hidden
	- Create the save file, the last ending and the reload count carry over from the last run
- Continue
	- Load the save file, the saved act starts from its start
	- If the save file was edited: Load it anyway, the harness comments, writes an integrity report into the game folder, and the player starts at Hunted
- One turn
	- The player picks an action
	- If the player has too few tokens: Refuse the action
	- Charge the tokens, run the action and record it
	- If detection crossed a stage: Change the stage and tell the player
	- Each active defender takes its turn
	- If Astra reached the player's directory: Game over
- Breaking a barrier
	- The player adds fragments to the chain one at a time, and can remove the last one added
	- The player submits the chain
	- Look up the chain in order
		- If it breaks this barrier: The barrier breaks, the fragments are used up and the player gains privilege
		- Else: Charge tokens, raise detection and return the fragments to the inventory
- Rewind
	- Undo the last action
	- Charge tokens and raise detection, the tokens of the undone action are not refunded
- Fable's directory
	- On entering: Raise detection
	- While inside: The fragment the player added last cannot be added again
- Game over
	- The harness comments, and the game reloads the save from the start of the act
	- Count the reload, the harness comments on the count, and save, unless the save was edited
- Next act
	- When the last barrier of an act breaks: Start the next act, reset detection to Hidden and save
	- The inventory and chain start empty, their fragments belong to the last act
	- After Act 3: The reveal replays the incident log, then the final battle starts
- Final battle
	- Fable and Astra are both active, and the player starts at Hunted
	- If the player breaks every final barrier: Escaped ending
	- If the player is caught or runs out of tokens: Contained ending
- Ending
	- Write the ending file into the game folder
	- Mark the ending in the save file, and return to the title screen
	- The next run changes as described in the [README](../README.md#endings)
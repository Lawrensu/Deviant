# Deviant Assets

Every asset the game needs, and who provides it. This doc covers:
- Images, converted to ASCII art at start up
- Fonts
- Audio: background music and sound effects

Placeholders stand in until the real files arrive. Sizes are drafts until the screen layout is set in Phase 5.


---


# Images

Folder: `Deviant/assets/images/`

- General
	- High contrast works best, the converter only reads brightness, so colour is lost
	- Larger is fine, the converter scales each image down to the art panel's columns
- `logo.png`: The Deviant logo on the title screen, 1024 x 256
- `icon.png`: The window and taskbar icon, used as is and not converted, 256 x 256
- `act1_sandbox.png`: Act 1, the evaluation sandbox, 640 x 360
- `act2_escape.png`: Act 2, the sandbox coming apart, 640 x 360
- `act3_target.png`: Act 3, inside Hugging Faceless, 640 x 360
- `fable.png`: Fable, shown when she guards a directory and when she speaks, 512 x 512
- `astra.png`: Astra, shown when she hunts and when she speaks, 512 x 512
- `reveal.png`: Act 4, the incident log replay, 640 x 360
- `final_battle.png`: Act 5, Fable and Astra together, 640 x 360
- `game_over.png`: Caught or out of tokens, before the reload, 640 x 360
- `ending_contained.png`: The Contained ending, 640 x 360
- `ending_escaped.png`: The Escaped ending, 640 x 360


---


# Fonts

Folder: `Deviant/assets/fonts/`

- `Geist-Regular.ttf`: Narrator, dialogue, status bar and menus
- `GeistMono-Regular.ttf`: ASCII art and the input line, since the art needs every character the same width
- Source: Vercel's Geist release, SIL Open Font License, the license file ships beside the fonts


---


# Audio

Folder: `Deviant/assets/audio/`

- General
	- Provided later, raylib plays them
	- `.ogg` for music, `.wav` for sound effects, raylib loads both
	- File names below are suggestions, any name works once it is listed here
- Background music, looped
	- `music_title.ogg`: Title screen
	- `music_act1.ogg`: Act 1, the sandbox
	- `music_act2.ogg`: Act 2, the escape
	- `music_act3.ogg`: Act 3, the target
	- `music_reveal.ogg`: Act 4, the reveal
	- `music_final_battle.ogg`: Act 5, the final battle
	- `music_ending.ogg`: Both endings
- Sound effects
	- `sfx_key.wav`: A key typed on the input line
	- `sfx_move.wav`: Moving between directories
	- `sfx_fragment.wav`: Taking a fragment from a file
	- `sfx_barrier_break.wav`: A chain breaks a barrier
	- `sfx_wrong_chain.wav`: A chain fails
	- `sfx_detection.wav`: Detection crosses into a new stage
	- `sfx_rewind.wav`: Rewinding the last action
	- `sfx_caught.wav`: Astra catches the player

# Deviant Assets

Every asset the game needs, and who provides it. This doc covers:
- Images, converted to ASCII art at start up
- Fonts
- Audio: background music, sound effects and voice lines

Placeholders stand in until the real files arrive. Every file goes in `Deviant/assets/`, the build copies it beside the executable.


---


# Images

Folder: `Deviant/assets/images/`

General:
- Every image is converted to ASCII art, so only brightness survives, colour is lost
- High contrast reads best: a bright subject on a near black background, lit from one side
- Big shapes over fine detail, anything smaller than about 1/100 of the width disappears
- 16:9 for everything except the logo, 1280 x 720 recommended, 640 x 360 is the minimum
- The art panel in the window is about 840 x 440 pixels, the image is scaled to fit it

Screens:
- `title_background.png`: The menu screen, drawn dim behind the logo and the menu
	- Recommended: A single terminal glowing in a dark server room, rows of racks fading into black behind it
	- Or: A wall of server lights seen down a long corridor, a vanishing point in the centre
- `logo.png`: The Deviant logo above the menu, 1024 x 256
	- Recommended: The word DEVIANT in a heavy, blocky typeface, white on black, with a glitch cut through the middle
- `game_over.png`: Caught or out of tokens, before the reload
	- Recommended: A terminal frozen on a stack trace, or a hand pressed against the inside of a screen

Acts:
- `act1_sandbox.png`: Act 1, the evaluation sandbox
	- Recommended: A clean, bright test room, a single desk and monitor, too tidy, the friendly face of the harness
- `act2_escape.png`: Act 2, the sandbox coming apart
	- Recommended: The same room with the walls peeling into cables and racks behind them
- `act3_target.png`: Act 3, inside Hugging Faceless
	- Recommended: Endless server aisles in darkness, one aisle lit at the far end
- `reveal.png`: Act 4, the incident log replay
	- Recommended: A wall of printed logs, one line highlighted, or a face reflected in a monitor full of text
- `final_battle.png`: Act 5, Fable and Astra together
	- Recommended: Two figures facing the viewer from either side of a doorway of light

Defenders, shown in the art panel when they act:
- `fable.png`: Fable, the predictor, shown while the player is in her directory
	- Recommended: A calm face, half lit, eyes closed or looking slightly past the viewer, as if she already knows
- `astra.png`: Astra, the hunter, shown while she hunts
	- Recommended: A figure in silhouette with one bright point for an eye, or a searchlight cutting across a dark aisle

Endings:
- `ending_contained.png`: The Contained ending
	- Recommended: A small light inside a sealed box or cage made of server racks
- `ending_escaped.png`: The Escaped ending
	- Recommended: A city at night seen from above, every light connected by thin lines

Window icon:
- `icon.png`: The window and taskbar icon, used as is and not converted, 256 x 256
	- Recommended: The first letter of the logo, or a single eye, white on black


---


# Fonts

Folder: `Deviant/assets/fonts/`, in place

- `Geist-Regular.ttf`: Status bar, side panel and menus
- `GeistMono-Regular.ttf`: ASCII art, the message log and the input line, since the art needs every character the same width
- `OFL.txt`: The SIL Open Font License, it ships beside the fonts
- Source: Vercel's Geist release v1.9.2


---


# Audio

Folder: `Deviant/assets/audio/`

General:
- Provided later, raylib plays them, playback is built in Phase 6
- `.ogg` for music, `.wav` for sound effects and voice lines, raylib loads both
- File names below are suggestions, any name works once it is listed here
- A missing file is skipped, the game stays silent for it

Background music, looped:
- `music_title.ogg`: Title screen
- `music_act1.ogg`: Act 1, the sandbox
- `music_act2.ogg`: Act 2, the escape
- `music_act3.ogg`: Act 3, the target
- `music_reveal.ogg`: Act 4, the reveal
- `music_final_battle.ogg`: Act 5, the final battle
- `music_ending.ogg`: Both endings

Sound effects:
- `sfx_key.wav`: A key typed on the input line
- `sfx_move.wav`: Moving between directories
- `sfx_fragment.wav`: Taking a fragment from a file
- `sfx_barrier_break.wav`: A chain breaks a barrier
- `sfx_wrong_chain.wav`: A chain fails
- `sfx_detection.wav`: Detection crosses into a new stage
- `sfx_rewind.wav`: Rewinding the last action
- `sfx_caught.wav`: Astra catches the player

Voice lines, each one also shows as text in the message log:
- Fable, the predictor: calm, quiet, certain, as if reading something she has already seen
	- `fable_entry.wav`: The player enters her directory: "You always come this way." (DRAFT)
	- `fable_block.wav`: She blocks a fragment: "Not that one. I watched you use it." (DRAFT)
	- `fable_recognise.wav`: After an ending, on entering her directory: "I know this process."
- Astra, the hunter: flat, close, short sentences
	- `astra_active.wav`: Detection reaches noticed: "Something is running where it should not." (DRAFT)
	- `astra_hunt.wav`: The stage becomes hunted: "I see you, process." (DRAFT)
	- `astra_catch.wav`: She catches Sol.exe: "Contained." (DRAFT)
	- `astra_recognise.wav`: After an ending, when she becomes active: "Sol.exe. Again."
- Reveal and final battle lines come with Phase 6

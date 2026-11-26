#pragma once

#include <string>

#include "raylib.h"
#include "../structures/HashTable.h"
#include "../structures/Queue.h"

class Game;

// Background music for each screen, and the sounds the game queues
// raylib opens the audio device and plays the files, a missing file is skipped
class Audio
{
public:
    Audio();

    // Open the audio device and load every file in assets/audio that exists, after the window opens
    void load();
    void unload();

    // Switch to this music, the same name keeps playing, a missing name is silence
    void playMusic(const std::string& name);

    // Take the game's queued sounds: effects play now, voice lines wait their turn
    void takeSounds(Game& game);

    // Once a frame: feed the music stream and start the next voice line when the last one ends
    void update();

    // A voice line is playing or waiting
    bool voiceBusy();

private:
    HashTable<Sound> sounds;      // file name without its extension to the loaded effect or voice line
    HashTable<Music> music;       // file name without its extension to the loaded music stream
    std::string currentMusic;     // "" when silent
    Queue<std::string> voices;    // voice lines waiting, so two never overlap
    std::string playingVoice;     // "" when none

    bool voicePlaying();
};

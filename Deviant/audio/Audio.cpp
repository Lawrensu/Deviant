#include "Audio.h"
#include "../game/Game.h"
#include "../game/Sandbox.h"

// Every file the game can play, listed in notes/assets.md
const int MUSIC_COUNT = 7;
const char* MUSIC_NAMES[MUSIC_COUNT] = {
    "music_title", "music_act1", "music_act2", "music_act3", "music_reveal", "music_final_battle", "music_ending"
};

const int SOUND_COUNT = 23;
const char* SOUND_NAMES[SOUND_COUNT] = {
    "sfx_key", "sfx_move", "sfx_fragment", "sfx_barrier_break", "sfx_wrong_chain", "sfx_detection", "sfx_rewind", "sfx_caught",
    "fable_entry", "fable_block", "fable_recognise", "fable_reveal", "fable_truth", "fable_contained", "fable_escaped",
    "astra_active", "astra_hunt", "astra_catch", "astra_recognise", "astra_reveal", "astra_truth", "astra_contained", "astra_escaped"
};

// The full path of a file in the audio folder beside the executable
static std::string audioPath(const std::string& fileName)
{
    return Sandbox::gameFolder() + "/assets/audio/" + fileName;
}

Audio::Audio() : currentMusic(""), playingVoice("") {}

void Audio::load()
{
    InitAudioDevice();

    // Hash table: each file that exists, found by its name when the game asks for it
    for (int i = 0; i < MUSIC_COUNT; i++)
    {
        std::string path = audioPath(std::string(MUSIC_NAMES[i]) + ".ogg");

        if (FileExists(path.c_str()))
        {
            music.insert(MUSIC_NAMES[i], LoadMusicStream(path.c_str()));
        }
    }

    for (int i = 0; i < SOUND_COUNT; i++)
    {
        std::string path = audioPath(std::string(SOUND_NAMES[i]) + ".wav");

        if (FileExists(path.c_str()))
        {
            sounds.insert(SOUND_NAMES[i], LoadSound(path.c_str()));
        }
    }
}

void Audio::unload()
{
    for (int i = 0; i < MUSIC_COUNT; i++)
    {
        Music* stream = music.find(MUSIC_NAMES[i]);

        if (stream != nullptr)
        {
            UnloadMusicStream(*stream);
        }
    }

    for (int i = 0; i < SOUND_COUNT; i++)
    {
        Sound* sound = sounds.find(SOUND_NAMES[i]);

        if (sound != nullptr)
        {
            UnloadSound(*sound);
        }
    }

    CloseAudioDevice();
}

void Audio::playMusic(const std::string& name)
{
    if (name == currentMusic)
    {
        return;
    }

    Music* playing = music.find(currentMusic);

    if (playing != nullptr)
    {
        StopMusicStream(*playing);
    }

    currentMusic = name;
    Music* next = music.find(name);

    if (next != nullptr)
    {
        PlayMusicStream(*next);
    }
}

void Audio::takeSounds(Game& game)
{
    // Queue: the game's sounds in the order they happened
    while (!game.sounds.isEmpty())
    {
        std::string name = game.sounds.dequeue();

        // Effects start "sfx_" and play at once, every other name is a voice line
        if (name.rfind("sfx_", 0) == 0)
        {
            Sound* effect = sounds.find(name);

            if (effect != nullptr)
            {
                PlaySound(*effect);
            }
        }
        else
        {
            voices.enqueue(name);
        }
    }
}

void Audio::update()
{
    Music* playing = music.find(currentMusic);

    if (playing != nullptr)
    {
        UpdateMusicStream(*playing);
    }

    if (voicePlaying())
    {
        return;
    }

    playingVoice = "";

    // Queue: the next voice line, a missing file is skipped
    while (!voices.isEmpty())
    {
        std::string name = voices.dequeue();
        Sound* voice = sounds.find(name);

        if (voice != nullptr)
        {
            PlaySound(*voice);
            playingVoice = name;
            return;
        }
    }
}

bool Audio::voiceBusy()
{
    if (voicePlaying())
    {
        return true;
    }

    return !voices.isEmpty();
}

bool Audio::voicePlaying()
{
    Sound* voice = sounds.find(playingVoice);

    if (voice == nullptr)
    {
        return false;
    }

    return IsSoundPlaying(*voice);
}

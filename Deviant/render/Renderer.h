#pragma once

#include <string>

#include "raylib.h"
#include "AsciiArt.h"
#include "../structures/SinglyLinkedList.h"

class Game;

const int LOG_LINES = 10;   // message lines kept and shown

// Draws every screen in the raylib window: art panel, status bar, side panel, message log, input line
class Renderer
{
public:
    std::string typing;   // the line being typed, moved to Game::input on Enter

    Renderer();

    // Fonts and art, after the window opens
    void load();
    void unload();

    // Move the game's new messages into the log, the oldest lines drop off
    void takeMessages(Game& game);

    void drawTitle(const std::string& lastEnding, bool hasSave);
    void draw(const Game& game);

private:
    Font textFont;    // Geist, status bar, side panel and menus
    Font monoFont;    // Geist Mono, message log and input line
    Font artFont;     // Geist Mono, small, for the ASCII art
    float artCharWidth;

    AsciiConverter converter;
    AsciiArt titleArt;
    AsciiArt logoArt;
    AsciiArt gameOverArt;
    AsciiArt fableArt;
    AsciiArt astraArt;
    AsciiArt actArt[6];   // by act number, 0 is unused

    SinglyLinkedList<std::string> log;

    AsciiArt loadArt(const std::string& fileName, float maxWidth, float maxHeight);
    void drawArt(const AsciiArt& art, float x, float y, float width, float height, Color color);
    void drawText(const std::string& text, float x, float y, Color color);
    void drawStatus(const Game& game);
    void drawSidePanel(const Game& game);
    void drawLog();
    void drawInput(const Game& game);
    void addLogLine(const std::string& line);
};

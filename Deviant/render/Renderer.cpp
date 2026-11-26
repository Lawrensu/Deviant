#include "Renderer.h"
#include "../game/Game.h"
#include "../game/Sandbox.h"

// Layout of the 1280 x 720 window, in pixels
const float ART_X = 16;
const float ART_Y = 48;
const float ART_WIDTH = 840;
const float ART_HEIGHT = 420;
const float SIDE_X = 880;
const float LOG_Y = 482;
const float LOG_LINE_HEIGHT = 18;
const float INPUT_Y = 676;

const float TEXT_SIZE = 20;
const float MONO_SIZE = 16;
const float ART_SIZE = 10;
const int LOG_COLUMNS = 128;   // longer messages wrap onto the next line

const Color BACKGROUND = { 8, 8, 8, 255 };
const Color DIM = { 90, 90, 90, 255 };
const Color TEXT = { 200, 200, 200, 255 };
const Color ART = { 170, 170, 170, 255 };

// The full path of a file in the assets folder beside the executable
static std::string assetPath(const std::string& folder, const std::string& fileName)
{
    return Sandbox::gameFolder() + "/assets/" + folder + "/" + fileName;
}

// The colour of each detection stage
static Color stageColor(const std::string& stage)
{
    if (stage == "hunted")
    {
        return Color{ 220, 60, 60, 255 };
    }

    if (stage == "noticed")
    {
        return Color{ 230, 190, 60, 255 };
    }

    return Color{ 90, 200, 120, 255 };
}

Renderer::Renderer() : textFont(), monoFont(), artFont(), artCharWidth(6) {}

void Renderer::load()
{
    // A missing font falls back to raylib's default font
    std::string text = assetPath("fonts", "Geist-Regular.ttf");
    std::string mono = assetPath("fonts", "GeistMono-Regular.ttf");

    textFont = LoadFontEx(text.c_str(), (int)TEXT_SIZE, nullptr, 0);
    monoFont = LoadFontEx(mono.c_str(), (int)MONO_SIZE, nullptr, 0);
    artFont = LoadFontEx(mono.c_str(), (int)ART_SIZE, nullptr, 0);

    artCharWidth = MeasureTextEx(artFont, "M", ART_SIZE, 0).x;

    // Every image is converted once, here
    titleArt = loadArt("title_background.png", 1280, 720);
    logoArt = loadArt("logo.png", 900, 160);
    gameOverArt = loadArt("game_over.png", ART_WIDTH, ART_HEIGHT);
    fableArt = loadArt("fable.png", ART_WIDTH, ART_HEIGHT);
    astraArt = loadArt("astra.png", ART_WIDTH, ART_HEIGHT);
    actArt[1] = loadArt("act1_sandbox.png", ART_WIDTH, ART_HEIGHT);
    actArt[2] = loadArt("act2_escape.png", ART_WIDTH, ART_HEIGHT);
    actArt[3] = loadArt("act3_target.png", ART_WIDTH, ART_HEIGHT);
    actArt[4] = loadArt("reveal.png", ART_WIDTH, ART_HEIGHT);
    actArt[5] = loadArt("final_battle.png", ART_WIDTH, ART_HEIGHT);
}

void Renderer::unload()
{
    UnloadFont(textFont);
    UnloadFont(monoFont);
    UnloadFont(artFont);
}

AsciiArt Renderer::loadArt(const std::string& fileName, float maxWidth, float maxHeight)
{
    std::string path = assetPath("images", fileName);
    int maxColumns = (int)(maxWidth / artCharWidth);
    int maxRows = (int)(maxHeight / ART_SIZE);

    if (maxRows > MAX_ROWS)
    {
        maxRows = MAX_ROWS;
    }

    AsciiArt art = converter.convert(path.c_str(), maxColumns);

    // Too tall for the space: convert again with fewer columns so the height fits
    if (art.rows > maxRows)
    {
        int columns = maxColumns * maxRows / art.rows;
        art = converter.convert(path.c_str(), columns);
    }

    return art;
}

void Renderer::takeMessages(Game& game)
{
    while (!game.messages.isEmpty())
    {
        std::string message = game.messages.dequeue();

        // Wrap a long message at the last space that fits, or cut it hard when there is no space
        while ((int)message.size() > LOG_COLUMNS)
        {
            size_t cut = message.rfind(' ', LOG_COLUMNS);
            size_t rest = cut + 1;

            if (cut == std::string::npos)
            {
                cut = LOG_COLUMNS;
                rest = LOG_COLUMNS;
            }

            addLogLine(message.substr(0, cut));
            message = "  " + message.substr(rest);
        }

        addLogLine(message);
    }
}

void Renderer::addLogLine(const std::string& line)
{
    log.pushBack(line);

    // Singly linked list: the oldest line is the head, drop it once the log is full
    if (log.size() > LOG_LINES)
    {
        log.remove(*log.begin());
    }
}

void Renderer::drawTitle(const std::string& lastEnding, bool hasSave)
{
    ClearBackground(BACKGROUND);

    // Background dim so the menu stays readable
    drawArt(titleArt, 0, 0, 1280, 720, Color{ 45, 45, 45, 255 });
    drawArt(logoArt, 190, 120, 900, 160, RAYWHITE);

    float y = 380;
    drawText("Last ending: " + lastEnding, 540, y, DIM);
    y = y + 50;

    if (hasSave)
    {
        drawText("C   Continue", 540, y, TEXT);
        drawText("N   New game", 540, y + 32, TEXT);
    }
    else
    {
        drawText("Enter   New game", 540, y, TEXT);
    }

    drawText("Esc   Quit", 540, y + 64, DIM);
}

void Renderer::draw(const Game& game)
{
    ClearBackground(BACKGROUND);
    drawStatus(game);

    // Art panel: Fable while the player is in her directory, Astra while hunting, else the act's scene
    bool inFableDirectory = false;

    if (game.player.location != nullptr)
    {
        inFableDirectory = game.player.location->guardedByFable;
    }

    if (game.state == "game_over")
    {
        drawArt(gameOverArt, ART_X, ART_Y, ART_WIDTH, ART_HEIGHT, Color{ 200, 70, 70, 255 });
    }
    else if (inFableDirectory)
    {
        drawArt(fableArt, ART_X, ART_Y, ART_WIDTH, ART_HEIGHT, Color{ 190, 200, 230, 255 });
    }
    else if (game.detectionStage == "hunted")
    {
        drawArt(astraArt, ART_X, ART_Y, ART_WIDTH, ART_HEIGHT, Color{ 220, 90, 90, 255 });
    }
    else
    {
        // Acts 1 to 5 each have a scene
        if (game.act >= 1)
        {
            if (game.act <= 5)
            {
                drawArt(actArt[game.act], ART_X, ART_Y, ART_WIDTH, ART_HEIGHT, ART);
            }
        }
    }

    DrawRectangleLines((int)ART_X - 4, (int)ART_Y - 4, (int)ART_WIDTH + 8, (int)ART_HEIGHT + 8, DIM);

    drawSidePanel(game);
    drawLog();

    // Phase 6 adds the reveal, final battle and ending screens, until then they use this layout
    if (game.state == "game_over")
    {
        DrawTextEx(monoFont, "Press Enter to reload", Vector2{ ART_X, INPUT_Y }, MONO_SIZE, 0, Color{ 220, 90, 90, 255 });
    }
    else
    {
        drawInput(game);
    }
}

void Renderer::drawArt(const AsciiArt& art, float x, float y, float width, float height, Color color)
{
    // A missing image shows its gap instead
    if (art.rows == 0)
    {
        drawText("[image missing]", x + 8, y + 8, DIM);
        return;
    }

    // Centre the art in its space
    float artWidth = art.columns * artCharWidth;
    float artHeight = art.rows * ART_SIZE;
    float left = x + (width - artWidth) / 2;
    float top = y + (height - artHeight) / 2;

    for (int row = 0; row < art.rows; row++)
    {
        DrawTextEx(artFont, art.lines[row].c_str(), Vector2{ left, top + row * ART_SIZE }, ART_SIZE, 0, color);
    }
}

void Renderer::drawText(const std::string& text, float x, float y, Color color)
{
    DrawTextEx(textFont, text.c_str(), Vector2{ x, y }, TEXT_SIZE, 0, color);
}

void Renderer::drawStatus(const Game& game)
{
    std::string left = "ACT " + std::to_string(game.act) + "     TOKENS " + std::to_string(game.tokens) +
        "     PRIVILEGE " + std::to_string(game.privilege) + "     DETECTION " + std::to_string(game.detection);
    drawText(left, ART_X, 14, TEXT);

    // The stage in its own colour
    float stageX = ART_X + MeasureTextEx(textFont, left.c_str(), TEXT_SIZE, 0).x + 20;
    drawText(game.detectionStage, stageX, 14, stageColor(game.detectionStage));
}

void Renderer::drawSidePanel(const Game& game)
{
    float y = ART_Y;

    if (game.player.location != nullptr)
    {
        drawText("LOCATION", SIDE_X, y, DIM);
        drawText(game.player.location->path(), SIDE_X, y + 24, TEXT);
        y = y + 64;

        Barrier* barrier = game.player.location->barrier;

        if (barrier != nullptr)
        {
            drawText("BARRIER", SIDE_X, y, DIM);

            if (barrier->broken)
            {
                drawText(barrier->name + " (broken)", SIDE_X, y + 24, DIM);
            }
            else
            {
                drawText(barrier->name, SIDE_X, y + 24, TEXT);
            }

            y = y + 64;
        }
    }

    // Iterator: the inventory front to back
    drawText("INVENTORY", SIDE_X, y, DIM);
    y = y + 24;

    for (Fragment* fragment : game.inventory)
    {
        drawText(fragment->id + "  " + fragment->name, SIDE_X, y, TEXT);
        y = y + 24;
    }

    y = y + 16;

    // The chain bottom to top, the order the fragments were added
    drawText("CHAIN  " + std::to_string(game.chain.size()) + " of " + std::to_string(CHAIN_LIMIT), SIDE_X, y, DIM);
    y = y + 24;
    std::string chain = "";

    for (int i = 0; i < game.chain.size(); i++)
    {
        chain = chain + game.chain.at(i)->id + "  ";
    }

    drawText(chain, SIDE_X, y, TEXT);
    y = y + 40;

    if (game.astra.location != nullptr)
    {
        drawText("ASTRA", SIDE_X, y, DIM);
        drawText(game.astra.location->path(), SIDE_X, y + 24, stageColor(game.detectionStage));
        y = y + 64;
    }

    if (game.fable.blockedFragment != nullptr)
    {
        drawText("FABLE BLOCKS", SIDE_X, y, DIM);
        drawText(game.fable.blockedFragment->id, SIDE_X, y + 24, Color{ 190, 200, 230, 255 });
    }
}

void Renderer::drawLog()
{
    float y = LOG_Y;

    // Iterator: the log oldest first
    for (const std::string& line : log)
    {
        DrawTextEx(monoFont, line.c_str(), Vector2{ ART_X, y }, MONO_SIZE, 0, TEXT);
        y = y + LOG_LINE_HEIGHT;
    }
}

void Renderer::drawInput(const Game& game)
{
    // The cursor blinks once a second
    std::string line = "> " + typing;

    if ((int)(GetTime() * 2) % 2 == 0)
    {
        line = line + "_";
    }

    // Dim while the defenders take their turns
    Color color = DIM;

    if (!game.turns.isEmpty())
    {
        if (game.turns.front() == &game.player)
        {
            color = TEXT;
        }
    }

    DrawTextEx(monoFont, line.c_str(), Vector2{ ART_X, INPUT_Y }, MONO_SIZE, 0, color);
}

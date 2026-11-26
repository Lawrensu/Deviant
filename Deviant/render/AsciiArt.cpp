#include "AsciiArt.h"

#include "raylib.h"

// A Geist Mono character is about 0.6 times as wide as it is tall,
// so a cell covers more pixels down than across and the art keeps its shape
const float CELL_ASPECT = 0.6f;

AsciiConverter::AsciiConverter() : ramp(" .:-=+*#%@") {}

AsciiArt AsciiConverter::convert(const char* imagePath, int columns)
{
    AsciiArt art;
    art.columns = 0;
    art.rows = 0;

    Image image = LoadImage(imagePath);

    // Missing or unreadable image
    if (image.data == nullptr)
    {
        return art;
    }

    if (columns <= 0)
    {
        UnloadImage(image);
        return art;
    }

    Color* pixels = LoadImageColors(image);

    // Size of one character cell in pixels
    float cellWidth = (float)image.width / columns;
    float cellHeight = cellWidth / CELL_ASPECT;
    int rows = (int)(image.height / cellHeight);

    if (rows > MAX_ROWS)
    {
        rows = MAX_ROWS;
    }

    for (int row = 0; row < rows; row++)
    {
        std::string line = "";
        int top = (int)(row * cellHeight);
        int bottom = (int)((row + 1) * cellHeight);

        // The last row may reach past the image
        if (bottom > image.height)
        {
            bottom = image.height;
        }

        for (int column = 0; column < columns; column++)
        {
            int left = (int)(column * cellWidth);
            int right = (int)((column + 1) * cellWidth);

            if (right > image.width)
            {
                right = image.width;
            }

            // Mean brightness of every pixel in the cell, transparent pixels count as dark
            long total = 0;
            int count = 0;

            for (int y = top; y < bottom; y++)
            {
                for (int x = left; x < right; x++)
                {
                    Color pixel = pixels[y * image.width + x];
                    int brightness = (pixel.r * 299 + pixel.g * 587 + pixel.b * 114) / 1000;
                    total = total + brightness * pixel.a / 255;
                    count++;
                }
            }

            int mean = 0;

            if (count > 0)
            {
                mean = (int)(total / count);
            }

            // 0 is the darkest character, the last one the brightest
            int index = mean * ((int)ramp.size() - 1) / 255;
            line = line + ramp[index];
        }

        art.lines[row] = line;
    }

    art.columns = columns;
    art.rows = rows;

    UnloadImageColors(pixels);
    UnloadImage(image);
    return art;
}

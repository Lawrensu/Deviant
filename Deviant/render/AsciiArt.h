#pragma once

#include <string>

const int MAX_ROWS = 80;

// ASCII art made from an image, one string per row
struct AsciiArt
{
    int columns;
    int rows;
    std::string lines[MAX_ROWS];
};

// Turns an image into ASCII art: each character cell's mean brightness picks a character
// raylib only loads the image and its pixels, the conversion is ours
class AsciiConverter
{
public:
    AsciiConverter();

    // rows is 0 when the image cannot be loaded or columns is 0
    AsciiArt convert(const char* imagePath, int columns);

private:
    std::string ramp;   // characters from dark to light
};

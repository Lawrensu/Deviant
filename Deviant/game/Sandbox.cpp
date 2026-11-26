#include "Sandbox.h"

#include <filesystem>
#include <fstream>

// windows.h only here, for the call that finds the executable's path, its names clash with raylib's
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

std::string Sandbox::gameFolder()
{
    char buffer[MAX_PATH];
    DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);

    // Path not found or too long, fall back to the current folder
    if (length == 0 || length == MAX_PATH)
    {
        return ".";
    }

    std::filesystem::path executable(buffer);
    return executable.parent_path().string();
}

bool Sandbox::writeFile(const std::string& name, const std::string& text)
{
    // A name with a folder in it could leave files/, refuse it
    if (name.find('/') != std::string::npos)
    {
        return false;
    }

    if (name.find('\\') != std::string::npos)
    {
        return false;
    }

    if (name.find("..") != std::string::npos)
    {
        return false;
    }

    // A drive, such as C:, would also leave the game folder
    if (name.find(':') != std::string::npos)
    {
        return false;
    }

    std::filesystem::path folder = std::filesystem::path(gameFolder()) / "files";

    // Create files/ when it is missing
    std::error_code error;
    std::filesystem::create_directories(folder, error);

    if (error)
    {
        return false;
    }

    std::ofstream file(folder / name);

    // Locked or read only
    if (!file.is_open())
    {
        return false;
    }

    file << text;
    return file.good();
}

bool Sandbox::exists(const std::string& name) const
{
    std::error_code error;
    return std::filesystem::exists(std::filesystem::path(gameFolder()) / "files" / name, error);
}

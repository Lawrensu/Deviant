#include "Directory.h"

Directory::Directory(const std::string& name)
    : name(name), parent(nullptr), childCount(0), fileCount(0), barrier(nullptr),
      locked(false), unlockCost(0), guardedByFable(false)
{
    for (int i = 0; i < MAX_CHILDREN; i++)
    {
        children[i] = nullptr;
    }

    for (int i = 0; i < MAX_FILES; i++)
    {
        files[i] = nullptr;
    }
}

Directory::~Directory()
{
    for (int i = 0; i < childCount; i++)
    {
        delete children[i];
    }

    for (int i = 0; i < fileCount; i++)
    {
        delete files[i];
    }
}

std::string Directory::path() const
{
    if (parent == nullptr)
    {
        return "/";
    }

    // Walk up the parent links, adding each name in front, until the root
    std::string result = "/" + name;
    Directory* current = parent;

    while (current->parent != nullptr)
    {
        result = "/" + current->name + result;
        current = current->parent;
    }

    return result;
}

DirectoryTree::DirectoryTree() : root(nullptr) {}

DirectoryTree::~DirectoryTree()
{
    delete root;
}

Directory* DirectoryTree::find(const std::string& path) const
{
    // Pre order search, compare each directory's path
    for (Directory* directory : *this)
    {
        if (directory->path() == path)
        {
            return directory;
        }
    }

    return nullptr;
}

Directory* DirectoryTree::nextStepToward(Directory* from, Directory* to) const
{
    if (from == to)
    {
        return from;
    }

    // Walk up from the target, reaching a child of from means the target is below that child, so it is the next step
    Directory* current = to;

    while (current != nullptr)
    {
        if (current->parent == from)
        {
            return current;
        }

        current = current->parent;
    }

    // The target is not below from, so the way there starts by going up
    return from->parent;
}

DirectoryTree::PreorderIterator DirectoryTree::begin() const
{
    return PreorderIterator(root);
}

DirectoryTree::PreorderIterator DirectoryTree::end() const
{
    return PreorderIterator(nullptr);
}

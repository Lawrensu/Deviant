#pragma once

#include <cstdlib>
#include <iostream>
#include <string>

#include "Item.h"
#include "../structures/Stack.h"

const int MAX_CHILDREN = 6;
const int MAX_FILES = 6;
const int TREE_STACK_LIMIT = 64;   // directories waiting to be visited during a traversal

// Tree: one node of the directory tree, it knows its parent and its children
class Directory
{
public:
    std::string name;
    Directory* parent;                    // nullptr for the root
    Directory* children[MAX_CHILDREN];
    int childCount;
    File* files[MAX_FILES];
    int fileCount;
    Barrier* barrier;                     // nullptr when the directory has none, owned by World
    bool locked;
    int unlockCost;                       // privilege needed to unlock
    bool guardedByFable;

    Directory(const std::string& name);

    // Frees the child directories and files, the whole subtree goes with it
    ~Directory();

    // Copies would share children and free them twice
    Directory(const Directory&) = delete;
    Directory& operator=(const Directory&) = delete;

    // Full path from the root, e.g. "/sandbox/logs"
    std::string path() const;
};

// Tree: the directory tree the player explores and Astra hunts through
class DirectoryTree
{
public:
    // Iterator: visits every directory in pre order, a parent before its children
    class PreorderIterator
    {
    public:
        PreorderIterator(Directory* start)
        {
            if (start != nullptr)
            {
                toVisit.push(start);
            }
        }

        Directory* operator*() const
        {
            return toVisit.top();
        }

        // Move past the top directory, and push its children last to first so the first child is on top
        PreorderIterator& operator++()
        {
            Directory* current = toVisit.pop();

            for (int i = current->childCount - 1; i >= 0; i--)
            {
                if (!toVisit.push(current->children[i]))
                {
                    std::cerr << "PreorderIterator: more than " << TREE_STACK_LIMIT << " directories waiting\n";
                    std::abort();
                }
            }

            return *this;
        }

        bool operator!=(const PreorderIterator& other) const
        {
            // Both finished
            if (toVisit.isEmpty() && other.toVisit.isEmpty())
            {
                return false;
            }

            // Only one finished
            if (toVisit.isEmpty() || other.toVisit.isEmpty())
            {
                return true;
            }

            return toVisit.top() != other.toVisit.top();
        }

    private:
        Stack<Directory*, TREE_STACK_LIMIT> toVisit;   // empty is end()
    };

    Directory* root;

    DirectoryTree();

    // Frees the root, which frees every directory below it
    ~DirectoryTree();

    // Copies would share the root and free it twice
    DirectoryTree(const DirectoryTree&) = delete;
    DirectoryTree& operator=(const DirectoryTree&) = delete;

    // The directory at this path, nullptr when there is none
    Directory* find(const std::string& path) const;

    // Astra's next directory on the way from one directory to another, through the parent links
    Directory* nextStepToward(Directory* from, Directory* to) const;

    PreorderIterator begin() const;
    PreorderIterator end() const;
};

#pragma once

#include "Directory.h"
#include "Item.h"
#include "../structures/HashTable.h"
#include "../structures/SinglyLinkedList.h"

// The world of one act: its directory tree, fragments, barriers and combinations
class World
{
public:
    DirectoryTree tree;
    HashTable<Barrier*> combinations;       // chain key in order, e.g. "f01+f02", to the barrier it breaks
    SinglyLinkedList<Fragment*> fragments;  // owns every fragment of the act, in a file, the inventory or the chain
    SinglyLinkedList<Barrier*> barriers;    // owns every barrier of the act

    ~World();

    // Directories, files, fragments, barriers and combinations of one act, replacing the last act's
    void buildAct(int act);

private:
    // Free the current act so the next one can be built
    void clear();

    void buildAct1();
};

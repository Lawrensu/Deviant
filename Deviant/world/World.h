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
    Directory* astraStart;                  // where Astra appears when she becomes active
    int tokenGrant;                         // tokens the harness adds when the act starts
    std::string outsideFileName;            // a file the act writes into files/, empty when it writes none
    std::string outsideFileText;

    World();

    ~World();

    // Directories, files, fragments, barriers and combinations of one act, replacing the last act's
    // Act 4 is the reveal and has no world, the tree stays empty
    void buildAct(int act);

private:
    // Free the current act so the next one can be built
    void clear();

    void buildAct1();
    void buildAct2();
    void buildAct3();
    void buildAct5();
};

#include "Demo.h"

#include <iostream>

// Every concept with a demo, and the phase that builds it
const int DEMO_COUNT = 10;

const char* DEMO_NAMES[DEMO_COUNT] = {
    "inheritance",
    "polymorphism",
    "hash_table",
    "singly_linked_list",
    "doubly_linked_list",
    "stack",
    "queue",
    "tree",
    "iterator",
    "command"
};

const int DEMO_PHASES[DEMO_COUNT] = { 2, 2, 1, 1, 1, 1, 1, 2, 1, 3 };

bool runDemo(const std::string& concept)
{
    for (int i = 0; i < DEMO_COUNT; i++)
    {
        if (concept == DEMO_NAMES[i])
        {
            // Placeholder until the concept's phase replaces it with the real demo
            std::cout << "Demo: " << concept << "\n";
            std::cout << "Not built yet, comes in Phase " << DEMO_PHASES[i] << "\n";
            return true;
        }
    }

    return false;
}

void printDemoUsage()
{
    std::cout << "Usage: Deviant.exe --demo <concept>\n";
    std::cout << "Concepts:\n";

    for (int i = 0; i < DEMO_COUNT; i++)
    {
        std::cout << "  " << DEMO_NAMES[i] << "\n";
    }
}

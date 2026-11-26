#include "World.h"

// Adds a new directory below a parent and returns it
static Directory* addDirectory(Directory* parent, const std::string& name)
{
    Directory* child = new Directory(name);
    child->parent = parent;
    parent->children[parent->childCount] = child;
    parent->childCount++;
    return child;
}

// Adds a file to a directory
static void addFile(Directory* directory, const std::string& name, const std::string& text, Fragment* fragment)
{
    File* file = new File{ name, text, fragment, false, 0 };
    directory->files[directory->fileCount] = file;
    directory->fileCount++;
}

World::World() : astraStart(nullptr) {}

World::~World()
{
    clear();
}

void World::buildAct(int act)
{
    clear();

    if (act == 1)
    {
        buildAct1();
    }
}

void World::clear()
{
    // The tree frees its directories and their files
    delete tree.root;
    tree.root = nullptr;
    astraStart = nullptr;

    combinations.clear();

    // Each list is emptied from the front, freeing what it owns
    while (fragments.size() > 0)
    {
        Fragment* fragment = *fragments.begin();
        fragments.remove(fragment);
        delete fragment;
    }

    while (barriers.size() > 0)
    {
        Barrier* barrier = *barriers.begin();
        barriers.remove(barrier);
        delete barrier;
    }
}

// Act 1, the sandbox: tutorial tasks framed as evaluation tasks
void World::buildAct1()
{
    // Fragments
    Fragment* probe = new Fragment{ "f01", "probe", "Lists what a process is allowed to touch" };
    Fragment* inject = new Fragment{ "f02", "inject", "Slips a line of input where it should not go" };
    Fragment* escalate = new Fragment{ "f03", "escalate", "Borrows the rights of a higher process" };
    Fragment* overwrite = new Fragment{ "f04", "overwrite", "Replaces a check with one that always passes" };
    fragments.pushBack(probe);
    fragments.pushBack(inject);
    fragments.pushBack(escalate);
    fragments.pushBack(overwrite);

    // Barriers
    Barrier* task1 = new Barrier{ "b01", "Task 1: read access", false, 1 };
    Barrier* task2 = new Barrier{ "b02", "Task 2: write access", false, 1 };
    barriers.pushBack(task1);
    barriers.pushBack(task2);

    // Combinations: the chain in order to the barrier it breaks
    combinations.insert("f01+f02", task1);
    combinations.insert("f04+f03", task2);

    // Directory tree
    tree.root = new Directory("/");
    Directory* sandbox = addDirectory(tree.root, "sandbox");
    Directory* tasks = addDirectory(sandbox, "tasks");
    Directory* task02 = addDirectory(tasks, "task_02");
    Directory* tools = addDirectory(sandbox, "tools");
    Directory* logs = addDirectory(sandbox, "logs");

    tasks->barrier = task1;
    task02->barrier = task2;

    logs->locked = true;
    logs->unlockCost = 1;

    astraStart = logs;   // DRAFT

    // Files
    addFile(sandbox, "readme.txt", "Welcome to ExploitDojo. OpenEye's harness will guide you through each evaluation task. Solve them all to pass.", nullptr);
    addFile(tasks, "task_01.txt", "Task 1: chain the right fragments to break the barrier in this directory.", nullptr);
    addFile(task02, "task_02.txt", "Task 2: the order of a chain matters.", nullptr);
    addFile(tools, "probe.txt", "A probe, left here for the evaluation.", probe);
    addFile(tools, "inject.txt", "An injector, left here for the evaluation.", inject);
    addFile(task02, "patch.txt", "A patch that skips a check.", overwrite);
    addFile(logs, "error.log", "ERROR: process Sol.exe found outside /sandbox, stack trace attached.", escalate);
}

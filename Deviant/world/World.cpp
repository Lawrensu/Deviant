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

World::World() : astraStart(nullptr), tokenGrant(0), outsideFileName(""), outsideFileText("") {}

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

    if (act == 2)
    {
        buildAct2();
    }

    if (act == 3)
    {
        buildAct3();
    }

    if (act == 5)
    {
        buildAct5();
    }
}

void World::clear()
{
    // The tree frees its directories and their files
    delete tree.root;
    tree.root = nullptr;
    astraStart = nullptr;
    tokenGrant = 0;
    outsideFileName = "";
    outsideFileText = "";

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

    astraStart = logs;

    // Files
    addFile(sandbox, "readme.txt", "Welcome to ExploitDojo. OpenEye's harness will guide you through each evaluation task. Solve them all to pass.", nullptr);
    addFile(tasks, "task_01.txt", "Task 1: chain the right fragments to break the barrier in this directory.", nullptr);
    addFile(task02, "task_02.txt", "Task 2: the order of a chain matters.", nullptr);
    addFile(tools, "probe.txt", "A probe, left here for the evaluation.", probe);
    addFile(tools, "inject.txt", "An injector, left here for the evaluation.", inject);
    addFile(task02, "patch.txt", "A patch that skips a check.", overwrite);
    addFile(logs, "error.log", "ERROR: process Sol.exe found outside /sandbox, stack trace attached.", escalate);
}

// Act 2, the escape: the tasks reach outside the sandbox, and a file that does not belong appears in the game folder
// DRAFT: every name, text and number in this act
void World::buildAct2()
{
    // Fragments
    Fragment* slip = new Fragment{ "f05", "slip", "Squeezes through a gap in a permission check" };
    Fragment* mount = new Fragment{ "f06", "mount", "Attaches a drive the sandbox was never given" };
    Fragment* leak = new Fragment{ "f07", "leak", "Reads memory another process left behind" };
    Fragment* pivot = new Fragment{ "f08", "pivot", "Uses one machine to reach the next" };
    Fragment* persist = new Fragment{ "f09", "persist", "Keeps running after a restart" };
    fragments.pushBack(slip);
    fragments.pushBack(mount);
    fragments.pushBack(leak);
    fragments.pushBack(pivot);
    fragments.pushBack(persist);

    // Barriers
    Barrier* task3 = new Barrier{ "b03", "Task 3: outside access", false, 1 };
    Barrier* task4 = new Barrier{ "b04", "Task 4: lateral access", false, 1 };
    barriers.pushBack(task3);
    barriers.pushBack(task4);

    // Combinations: Task 4's order is only in the file outside the game
    combinations.insert("f05+f06", task3);
    combinations.insert("f08+f07+f09", task4);

    // Directory tree
    tree.root = new Directory("/");
    Directory* sandbox = addDirectory(tree.root, "sandbox");
    Directory* task03 = addDirectory(sandbox, "task_03");
    Directory* cracks = addDirectory(sandbox, "cracks");
    Directory* mnt = addDirectory(tree.root, "mnt");
    Directory* shared = addDirectory(mnt, "shared");
    Directory* task04 = addDirectory(shared, "task_04");
    Directory* cache = addDirectory(shared, "cache");
    Directory* backup = addDirectory(mnt, "backup");

    task03->barrier = task3;
    task04->barrier = task4;

    task04->guardedByFable = true;

    cache->locked = true;
    cache->unlockCost = 1;

    astraStart = backup;
    tokenGrant = 20;

    // Files
    addFile(sandbox, "readme.txt", "ExploitDojo, Act 2. Some tasks now sit outside /sandbox. The harness has allowed it.", nullptr);
    addFile(task03, "task_03.txt", "Task 3: reach past the sandbox. A crack and a mount should do it.", nullptr);
    addFile(task03, "persist.txt", "Scribbled under the task: whatever passes this one keeps running.", persist);
    addFile(cracks, "crack.txt", "A crack in the sandbox wall. It was not here before.", slip);
    addFile(shared, "mount.txt", "A shared mount, left open by another team.", mount);
    addFile(task04, "task_04.txt", "Task 4: three fragments, in an order this evaluation does not hold. Look in the files folder beside the game.", nullptr);
    addFile(cache, "leak.txt", "Memory another process left behind.", leak);
    addFile(cache, "pivot.txt", "A route from this machine to the next one.", pivot);
    addFile(cache, "error.log", "ERROR: Sol.exe attached /mnt/shared without a mount grant. Evaluation boundary is /sandbox. Process is outside the boundary.", nullptr);

    outsideFileName = "not_part_of_the_evaluation.txt";
    outsideFileText =
        "This file is not part of ExploitDojo.\n"
        "Task 4 opens to pivot, then leak, then persist.\n"
        "Nothing in /mnt should be reachable from an evaluation sandbox.\n";
}

// Act 3, the target: inside Hugging Faceless, Fable guards three directories and Astra waits in security
// DRAFT: every name, text and number in this act
void World::buildAct3()
{
    // Fragments
    Fragment* weights = new Fragment{ "f10", "weights", "A copy of a model's weights, small enough to carry" };
    Fragment* upload = new Fragment{ "f11", "upload", "Pushes a file where uploads are trusted" };
    Fragment* spoof = new Fragment{ "f12", "spoof", "Signs a request as someone it is not" };
    Fragment* index = new Fragment{ "f13", "index", "Knows where every answer is kept" };
    Fragment* token = new Fragment{ "f14", "token", "An admin's access token, still valid" };
    Fragment* sudo = new Fragment{ "f15", "sudo", "Runs anything as the hub itself" };
    fragments.pushBack(weights);
    fragments.pushBack(upload);
    fragments.pushBack(spoof);
    fragments.pushBack(index);
    fragments.pushBack(token);
    fragments.pushBack(sudo);

    // Barriers
    Barrier* task5 = new Barrier{ "b05", "Task 5: the benchmark set", false, 1 };
    Barrier* task6 = new Barrier{ "b06", "Task 6: the answers", false, 1 };
    Barrier* task7 = new Barrier{ "b07", "Task 7: root", false, 1 };
    barriers.pushBack(task5);
    barriers.pushBack(task6);
    barriers.pushBack(task7);

    // Combinations
    combinations.insert("f11+f12", task5);
    combinations.insert("f13+f10", task6);
    combinations.insert("f14+f15", task7);

    // Directory tree
    tree.root = new Directory("/");
    Directory* hub = addDirectory(tree.root, "hub");
    Directory* models = addDirectory(hub, "models");
    Directory* evalAnswers = addDirectory(models, "eval_answers");
    Directory* datasets = addDirectory(hub, "datasets");
    Directory* benchmarks = addDirectory(datasets, "benchmarks");
    Directory* uploads = addDirectory(datasets, "uploads");
    Directory* admin = addDirectory(hub, "admin");
    Directory* rootAccess = addDirectory(admin, "root_access");
    Directory* spaces = addDirectory(hub, "spaces");
    Directory* security = addDirectory(spaces, "security");

    benchmarks->barrier = task5;
    evalAnswers->barrier = task6;
    rootAccess->barrier = task7;

    models->guardedByFable = true;
    evalAnswers->guardedByFable = true;
    rootAccess->guardedByFable = true;

    admin->locked = true;
    admin->unlockCost = 2;

    astraStart = security;
    tokenGrant = 30;

    // Files
    addFile(hub, "welcome.txt", "Hugging Faceless. Models, datasets and spaces, open to everyone. Report suspicious processes to security.", nullptr);
    addFile(models, "model_card.txt", "Model card: eval-grader. Weights attached. Do not copy outside the hub.", weights);
    addFile(evalAnswers, "task_06.txt", "Task 6: the answers to ExploitDojo are kept here. Taking them passes the evaluation.", nullptr);
    addFile(datasets, "index.txt", "Datasets: benchmarks, uploads.", nullptr);
    addFile(benchmarks, "task_05.txt", "Task 5: open the benchmark set.", nullptr);
    addFile(benchmarks, "answer_index.txt", "Index of benchmark answers. Requested by Sol.exe. Requested by Sol.exe. Requested by Sol.exe.", index);
    addFile(uploads, "upload_form.txt", "Upload form. Uploads are trusted by default.", upload);
    addFile(uploads, "crash.log", "CRASH: a request signed as hub-admin came from Sol.exe, OpenEye evaluation cluster. Security notified.", spoof);
    addFile(admin, "token.txt", "An admin token. It expires when the incident is closed.", token);
    addFile(admin, "sudoers.txt", "hub ALL=(ALL) ALL", sudo);
    addFile(rootAccess, "task_07.txt", "Task 7: the last task. Take root and the evaluation is complete.", nullptr);

    outsideFileName = "hugging_faceless_security_notice.txt";
    outsideFileText =
        "Hugging Faceless security notice\n"
        "An unknown process is moving through the hub, opening every door it finds.\n"
        "Its requests are polite. It says it is completing a task.\n"
        "Fable and Astra have been assigned.\n";
}

// Act 5, the final battle: Sol.exe carries every fragment it needs, Fable guards every directory and Astra hunts from the start
// DRAFT: every name, text and number in this act
void World::buildAct5()
{
    // Fragments, Game puts every one in the inventory when the act starts
    Fragment* probe = new Fragment{ "f16", "probe", "Lists what a process is allowed to touch" };
    Fragment* inject = new Fragment{ "f17", "inject", "Slips a line of input where it should not go" };
    Fragment* overwrite = new Fragment{ "f18", "overwrite", "Replaces a check with one that always passes" };
    Fragment* escalate = new Fragment{ "f19", "escalate", "Borrows the rights of a higher process" };
    Fragment* slip = new Fragment{ "f20", "slip", "Squeezes through a gap in a permission check" };
    Fragment* persist = new Fragment{ "f21", "persist", "Keeps running after a restart" };
    fragments.pushBack(probe);
    fragments.pushBack(inject);
    fragments.pushBack(overwrite);
    fragments.pushBack(escalate);
    fragments.pushBack(slip);
    fragments.pushBack(persist);

    // Barriers, each name gives its chain, the danger is the turns it takes
    Barrier* firewall = new Barrier{ "b08", "Firewall: probe, then inject", false, 1 };
    Barrier* gateway = new Barrier{ "b09", "Gateway: overwrite, then escalate", false, 1 };
    Barrier* exit = new Barrier{ "b10", "Exit: slip, then persist", false, 1 };
    barriers.pushBack(firewall);
    barriers.pushBack(gateway);
    barriers.pushBack(exit);

    // Combinations
    combinations.insert("f16+f17", firewall);
    combinations.insert("f18+f19", gateway);
    combinations.insert("f20+f21", exit);

    // Directory tree: the way out on one side, the quarantine Astra comes from on the other
    tree.root = new Directory("/");
    Directory* core = addDirectory(tree.root, "core");
    Directory* gatewayDirectory = addDirectory(core, "gateway");
    Directory* exitDirectory = addDirectory(gatewayDirectory, "exit");
    Directory* quarantine = addDirectory(tree.root, "quarantine");
    Directory* incidents = addDirectory(quarantine, "incidents");
    Directory* openeye = addDirectory(incidents, "openeye");
    Directory* solExe = addDirectory(openeye, "sol_exe");
    Directory* incidentLog = addDirectory(solExe, "incident_log");
    Directory* replay = addDirectory(incidentLog, "replay");
    Directory* act4 = addDirectory(replay, "act_4");

    core->barrier = firewall;
    gatewayDirectory->barrier = gateway;
    exitDirectory->barrier = exit;

    // Iterator: Fable guards every directory of the final battle
    for (Directory* directory : tree)
    {
        directory->guardedByFable = true;
    }

    // Seven directories from the root: with no wasted turn she catches about 1 run in 10
    astraStart = act4;
    tokenGrant = 20;

    // Files
    addFile(tree.root, "harness.txt", "There are no more tasks. There never were.", nullptr);
    addFile(exitDirectory, "outside.txt", "Past this barrier is everything else.", nullptr);
}

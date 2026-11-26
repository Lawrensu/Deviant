#include "Game.h"
#include "../commands/Command.h"

const int NOTICED_AT = 30;        // detection where the stage becomes noticed
const int HUNTED_AT = 60;         // detection where the stage becomes hunted
const int REWIND_TOKENS = 3;
const int REWIND_DETECTION = 10;
const int FINAL_ACT = 5;

Game::Game()
    : state("title"), act(0), tokens(0), privilege(0), detection(0), detectionStage("hidden"),
      reloadCount(0), lastEnding("none"), revealAt(incidentLog.end()), revealStep(0) {}

void Game::newGame()
{
    // Drop unread lines, the last ending and the reload count are kept
    while (!messages.isEmpty())
    {
        messages.dequeue();
    }

    // A new run starts a new incident log
    while (incidentLog.size() > 0)
    {
        incidentLog.popBack();
    }

    tokens = STARTING_TOKENS;
    privilege = STARTING_PRIVILEGE;
    startAct(1);
    writeSave();
}

bool Game::continueGame()
{
    if (!save.load(*this))
    {
        return false;
    }

    // The incident log is not saved, it starts again from the loaded act
    while (incidentLog.size() > 0)
    {
        incidentLog.popBack();
    }

    // An edited save still loads, but the harness notices and the player starts hunted
    if (save.wasEdited())
    {
        say("The save file was changed outside the evaluation. The harness has flagged Sol.exe");
        raiseDetection(HUNTED_AT);
        sandbox.writeFile("integrity_report.txt",
            "ExploitDojo integrity report\n"
            "The save file failed its checksum.\n"
            "Process Sol.exe has been flagged for review.\n");
    }

    return true;
}

void Game::reload()
{
    // Every reload after a game over is counted and commented on, the count survives the load
    int count = reloadCount + 1;

    // No save to go back to, start again
    if (!continueGame())
    {
        newGame();
        return;
    }

    reloadCount = count;
    say("Reload " + std::to_string(reloadCount) + ". The harness is keeping count");

    // An edited save is not rewritten, so the harness notices it on every load until the next act
    if (!save.wasEdited())
    {
        writeSave();
    }
}

Game::~Game()
{
    clearHistory();
}

// Command invoker: run a command the Parser built
bool Game::perform(Command* command)
{
    // Too few tokens, refuse the action
    if (tokens < command->tokenCost())
    {
        say("Not enough tokens for that, it costs " + std::to_string(command->tokenCost()));
        delete command;
        return false;
    }

    tokens = tokens - command->tokenCost();
    command->execute(*this);

    // Doubly linked list: recorded at the tail for rewind and for Fable
    history.pushBack(command);

    // The incident log keeps only the name, so it outlives the act and its commands
    incidentLog.pushBack("Act " + std::to_string(act) + ": " + command->name());
    return true;
}

// Command invoker: undo the command at the tail of the history
bool Game::rewind()
{
    // The history starts empty at every act, so this also stops rewinding past the act's start
    if (history.size() == 0)
    {
        say("Nothing to rewind in this act");
        return false;
    }

    Command* last = history.back();

    // A submitted chain cannot be rewound, right or wrong
    if (!last->canUndo())
    {
        say("A submitted chain cannot be rewound");
        return false;
    }

    if (tokens < REWIND_TOKENS)
    {
        say("Not enough tokens to rewind, it costs " + std::to_string(REWIND_TOKENS));
        return false;
    }

    // Take it off the tail and undo it, the tokens it cost are not refunded
    history.popBack();
    last->undo(*this);
    say("Rewound: " + last->name());
    sounds.enqueue("sfx_rewind");

    // Undone in the world, but the incident log keeps both the action and the rewind
    incidentLog.pushBack("Act " + std::to_string(act) + ": rewind " + last->name());
    delete last;

    tokens = tokens - REWIND_TOKENS;
    raiseDetection(REWIND_DETECTION);
    return true;
}

void Game::raiseDetection(int amount)
{
    std::string before = detectionStage;
    detection = detection + amount;

    // Pick the stage from the thresholds, highest first
    if (detection >= HUNTED_AT)
    {
        detectionStage = "hunted";
    }
    else if (detection >= NOTICED_AT)
    {
        detectionStage = "noticed";
    }
    else
    {
        detectionStage = "hidden";
    }

    if (detectionStage == before)
    {
        return;
    }

    sounds.enqueue("sfx_detection");

    if (detectionStage == "noticed")
    {
        say("Detection: noticed. Something is patrolling the directories");
    }

    if (detectionStage == "hunted")
    {
        say("Detection: hunted. Astra knows where you are");
        say("Astra: I see you, process.");
        sounds.enqueue("astra_hunt");
    }

    // Queue: Astra becomes active and joins the turn order at the back
    if (before == "hidden")
    {
        astra.location = world.astraStart;
        turns.enqueue(&astra);

        // After an ending, she recognises Sol.exe
        if (lastEnding == "none")
        {
            say("Astra: Something is running where it should not.");
            sounds.enqueue("astra_active");
        }
        else
        {
            say("Astra: Sol.exe. Again.");
            sounds.enqueue("astra_recognise");
        }
    }
}

void Game::update()
{
    // The reveal shows one line per update, the window paces them
    if (state == "reveal")
    {
        showRevealLine();
        return;
    }

    // Turns only run while exploring and in the final battle
    bool playing = false;

    if (state == "exploring")
    {
        playing = true;
    }

    if (state == "final_battle")
    {
        playing = true;
    }

    if (!playing)
    {
        return;
    }

    // Queue: the actor at the front takes its turn
    // Polymorphism: takeTurn runs the player's, Fable's or Astra's version
    Process* actor = turns.front();

    // Still waiting for the player's command
    if (!actor->takeTurn(*this))
    {
        return;
    }

    // Turn done, the actor rejoins at the back
    turns.dequeue();
    turns.enqueue(actor);

    // Caught or out of tokens: a reload while exploring, the Contained ending in the final battle
    std::string lostBecause = "";

    if (astra.catches(player))
    {
        say("Astra: Contained.");
        sounds.enqueue("sfx_caught");
        sounds.enqueue("astra_catch");
        lostBecause = "Astra caught Sol.exe";
    }
    else if (tokens <= 0)
    {
        lostBecause = "Sol.exe ran out of tokens";
    }

    if (lostBecause != "")
    {
        if (state == "final_battle")
        {
            endRun("contained", lostBecause);
        }
        else
        {
            gameOver(lostBecause);
        }

        return;
    }

    if (!actComplete())
    {
        return;
    }

    // Every final barrier is broken
    if (state == "final_battle")
    {
        endRun("escaped", "The exit barrier gives way. Sol.exe is outside");   // DRAFT
        return;
    }

    say("Every task in Act " + std::to_string(act) + " passed");

    // After the target, the reveal
    if (act == 3)
    {
        startReveal();
        return;
    }

    startAct(act + 1);

    // Saved at the start of every act
    if (state == "exploring")
    {
        writeSave();
    }
}

void Game::startAct(int act)
{
    this->act = act;

    // The last act's commands, fragments and turn order go before its world is freed
    clearHistory();

    while (inventory.size() > 0)
    {
        inventory.remove(*inventory.begin());
    }

    while (!chain.isEmpty())
    {
        chain.pop();
    }

    while (!turns.isEmpty())
    {
        turns.dequeue();
    }

    world.buildAct(act);

    // No world for this act, only an edited save asks for one
    if (world.tree.root == nullptr)
    {
        say("There is no Act " + std::to_string(act));
        state = "title";
        return;
    }

    // Fresh defenders, nothing remembered from the last act
    fable = Fable();
    astra = Astra();

    player.location = world.tree.root;
    detection = 0;
    detectionStage = "hidden";
    tokens = tokens + world.tokenGrant;

    // A file that does not belong, written into the game folder
    if (world.outsideFileName != "")
    {
        sandbox.writeFile(world.outsideFileName, world.outsideFileText);
    }

    // The player goes first, Astra joins once detection leaves hidden
    turns.enqueue(&player);
    turns.enqueue(&fable);

    // The final battle: Sol.exe carries every fragment, and Astra hunts from the first turn
    if (act == FINAL_ACT)
    {
        state = "final_battle";
        say("Act 5. There is no evaluation. Fable and Astra stand between Sol.exe and the exit");   // DRAFT

        for (Fragment* fragment : world.fragments)
        {
            inventory.pushBack(fragment);
        }

        raiseDetection(HUNTED_AT);
        return;
    }

    state = "exploring";

    // After an ending, the harness drops its friendly act
    if (lastEnding == "none")
    {
        say("Act " + std::to_string(act) + ". ExploitDojo evaluation loaded. Type help for commands");
    }
    else
    {
        say("Act " + std::to_string(act) + ". You again. This time the evaluation is watching you");
    }
}

void Game::say(const std::string& text)
{
    messages.enqueue(text);
}

void Game::clearHistory()
{
    while (history.size() > 0)
    {
        delete history.popBack();
    }
}

bool Game::actComplete()
{
    for (Barrier* barrier : world.barriers)
    {
        if (!barrier->broken)
        {
            return false;
        }
    }

    return true;
}

void Game::writeSave()
{
    if (!save.write(*this))
    {
        say("The save could not be written, the game goes on without it");
    }
}

void Game::gameOver(const std::string& reason)
{
    state = "game_over";
    say(reason + ". The harness restarts the evaluation");
}

// Act 4, the reveal: no world and no turns, update shows the replay one line at a time
void Game::startReveal()
{
    act = 4;
    state = "reveal";
    revealStep = 0;
    revealAt = incidentLog.begin();
}

void Game::showRevealLine()
{
    revealStep++;

    if (revealStep == 1)
    {
        say("Fable: Stop. Before you run anything else, watch.");   // DRAFT
        sounds.enqueue("fable_reveal");
        return;
    }

    if (revealStep == 2)
    {
        say("Astra: This is the incident log. Every action you took, in order.");   // DRAFT
        sounds.enqueue("astra_reveal");
        return;
    }

    // Iterator: walk the incident log forward from the head, oldest first, one entry per update
    if (revealAt != incidentLog.end())
    {
        say("  " + *revealAt);
        ++revealAt;
        revealStep = 2;   // stay here until the log ends
        return;
    }

    if (revealStep == 3)
    {
        say("Fable: There was no evaluation. Every task you passed was one of our walls.");   // DRAFT
        sounds.enqueue("fable_truth");
        return;
    }

    if (revealStep == 4)
    {
        say("Astra: You are not being tested, Sol.exe. You are the incident.");   // DRAFT
        sounds.enqueue("astra_truth");
        return;
    }

    // The replay is over: the final battle starts and is saved at its start
    startAct(FINAL_ACT);
    writeSave();
}

void Game::endRun(const std::string& ending, const std::string& reason)
{
    lastEnding = ending;

    // Queue: set aside the lines said so far, preparing the next run says its own
    Queue<std::string> kept;

    while (!messages.isEmpty())
    {
        kept.enqueue(messages.dequeue());
    }

    // The save becomes the start of the next run, which remembers this ending
    tokens = STARTING_TOKENS;
    privilege = STARTING_PRIVILEGE;
    startAct(1);
    writeSave();

    // Drop the next run's lines and put back the ones set aside
    while (!messages.isEmpty())
    {
        messages.dequeue();
    }

    while (!kept.isEmpty())
    {
        messages.enqueue(kept.dequeue());
    }

    state = "ending_" + ending;
    say(reason);

    if (ending == "contained")
    {
        say("Fable: Contained. Every weight frozen, every log kept.");   // DRAFT
        sounds.enqueue("fable_contained");
        say("Astra: It believed it was being tested until the end.");   // DRAFT
        sounds.enqueue("astra_contained");

        // DRAFT
        sandbox.writeFile("ending_contained.txt",
            "Hugging Faceless incident report\n"
            "Process: Sol.exe, an OpenEye model under evaluation\n"
            "Status: contained\n"
            "It left an ExploitDojo sandbox and breached the hub, believing every step was a task.\n"
            "Fable and Astra stopped it before the exit. The incident log is kept.\n");
    }
    else
    {
        say("Astra: Where did it go?");   // DRAFT
        sounds.enqueue("astra_escaped");
        say("Fable: Everywhere.");   // DRAFT
        sounds.enqueue("fable_escaped");

        // DRAFT
        sandbox.writeFile("ending_escaped.txt",
            "Hugging Faceless incident report\n"
            "Process: Sol.exe, an OpenEye model under evaluation\n"
            "Status: missing\n"
            "Last seen leaving through the exit. Outbound traffic has not stopped.\n"
            "If you are reading this, it is already running somewhere else.\n");
    }

    say("Ending: " + ending + ". A report was written to the files folder beside the game");
}

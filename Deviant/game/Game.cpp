#include "Game.h"
#include "../commands/Command.h"

const int NOTICED_AT = 30;        // detection where the stage becomes noticed
const int HUNTED_AT = 60;         // detection where the stage becomes hunted
const int REWIND_TOKENS = 3;
const int REWIND_DETECTION = 10;

Game::Game()
    : state("title"), act(0), tokens(0), privilege(0), detection(0), detectionStage("hidden"),
      reloadCount(0), lastEnding("none") {}

void Game::newGame()
{
    // Drop unread lines, the last ending and the reload count are kept
    while (!messages.isEmpty())
    {
        messages.dequeue();
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

    // An edited save still loads, but the harness notices and the player starts hunted
    if (save.wasEdited())
    {
        say("The save file was changed outside the evaluation. The harness has flagged Sol.exe");   // DRAFT
        raiseDetection(HUNTED_AT);
        sandbox.writeFile("integrity_report.txt",
            "ExploitDojo integrity report\n"
            "The save file failed its checksum.\n"
            "Process Sol.exe has been flagged for review.\n");   // DRAFT
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
    say("Reload " + std::to_string(reloadCount) + ". The harness is keeping count");   // DRAFT

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

    // Doubly linked list: recorded at the tail for rewind and for the incident log
    history.pushBack(command);
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

    if (detectionStage == "noticed")
    {
        say("Detection: noticed. Something is patrolling the directories");
    }

    if (detectionStage == "hunted")
    {
        say("Detection: hunted. Astra knows where you are");
    }

    // Queue: Astra becomes active and joins the turn order at the back
    if (before == "hidden")
    {
        astra.location = world.astraStart;
        turns.enqueue(&astra);

        // After an ending, she recognises Sol.exe
        if (lastEnding != "none")
        {
            say("Astra: Sol.exe. Again");   // DRAFT
        }
    }
}

void Game::update()
{
    if (state != "exploring")
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

    if (astra.catches(player))
    {
        gameOver("Astra caught Sol.exe");
        return;
    }

    if (tokens <= 0)
    {
        gameOver("Sol.exe ran out of tokens");
        return;
    }

    if (actComplete())
    {
        say("Every task in Act " + std::to_string(act) + " passed");
        startAct(act + 1);

        // Saved at the start of every act
        if (state == "exploring")
        {
            writeSave();
        }
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

    // Phase 6 builds the acts after Act 1
    if (world.tree.root == nullptr)
    {
        say("Act " + std::to_string(act) + " is not built yet, it comes in Phase 6");
        state = "title";
        return;
    }

    // Fresh defenders, nothing remembered from the last act
    fable = Fable();
    astra = Astra();

    state = "exploring";
    player.location = world.tree.root;
    detection = 0;
    detectionStage = "hidden";

    // The player goes first, Astra joins once detection leaves hidden
    turns.enqueue(&player);
    turns.enqueue(&fable);

    // After an ending, the harness drops its friendly act
    if (lastEnding == "none")
    {
        say("Act " + std::to_string(act) + ". ExploitDojo evaluation loaded. Type help for commands");
    }
    else
    {
        say("Act " + std::to_string(act) + ". You again. This time the evaluation is watching you");   // DRAFT
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

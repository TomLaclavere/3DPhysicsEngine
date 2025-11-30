#include "external/linenoise/hystory.hpp"
#include "external/linenoise/linenoise.h"
#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "precision.hpp"
#include "utilities/command.hpp"
#include "utilities/timer.hpp"
#include "world/config.hpp"
#include "world/physicsWorld.hpp"

#include <chrono>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

std::string completionFilename = "./src/external/linenoise/completion.txt";
const char* historyFilename    = "./src/external/linenoise/history.txt";

// ============================================================================
// Helpers
// ============================================================================
void listObjects(const PhysicsWorld& world)
{
    std::cout << "Objects (" << world.getObjectCount() << "):\n";
    for (size_t i = 0; i < world.getObjectCount(); ++i)
    {
        const Object* obj = world.getObject(i);
        if (obj)
            std::cout << "  [" << i << "] " << toString(obj->getType()) << " | pos=" << obj->getPosition()
                      << " | vel=" << obj->getVelocity() << " | fixed=" << (obj->isFixed() ? "True" : "False")
                      << "\n";
    }
}

void showObject(const PhysicsWorld& world, size_t id)
{
    const Object* obj = world.getObject(id);
    if (!obj)
    {
        std::cout << "No object with id " << id << "\n";
        return;
    }

    std::cout << "Object [" << id << "]\n"
              << "  Type: " << toString(obj->getType()) << "\n"
              << "  Position: " << obj->getPosition() << "\n"
              << "  Velocity: " << obj->getVelocity() << "\n"
              << "  Fixed: " << std::boolalpha << obj->isFixed() << "\n";
}

// ============================================================================
// Command enumeration for switch
// ============================================================================
enum class CommandType
{
    HELP,
    EXIT,
    START,
    STOP,
    RUN,
    INTEGRATE,
    PRINT,
    INIT,
    SET,
    ADD,
    LIST,
    SHOW,
    DEL,
    UNKNOWN
};

CommandType commandFromString(const std::string& action)
{
    if (action == "help" || action == "h")
        return CommandType::HELP;
    if (action == "exit" || action == "quit" || action == "q")
        return CommandType::EXIT;
    if (action == "start")
        return CommandType::START;
    if (action == "stop")
        return CommandType::STOP;
    if (action == "run")
        return CommandType::RUN;
    if (action == "integrate")
        return CommandType::INTEGRATE;
    if (action == "print")
        return CommandType::PRINT;
    if (action == "init")
        return CommandType::INIT;
    if (action == "set")
        return CommandType::SET;
    if (action == "add")
        return CommandType::ADD;
    if (action == "list")
        return CommandType::LIST;
    if (action == "show")
        return CommandType::SHOW;
    if (action == "del")
        return CommandType::DEL;
    return CommandType::UNKNOWN;
}

// ============================================================================
// Main
// ============================================================================
int main(int argc, char** argv)
{
    // Initialize history & completion helpers (wraps linenoise callbacks / loading)
    initHistoryAndCompletion(historyFilename, completionFilename);

    Timer totalTimer;

    // Load configuration
    Timer   configTimer;
    Config& config = Config::get();
    config.loadFromFile("src/config.yaml");
    config.overrideFromCommandLine(argc, argv);

    std::cout << "----------------------------------------\n";
    std::cout << "Simulation Parameters:\n";
    std::cout << "  Gravity: " << config.getGravity() << " m/s²\n";
    std::cout << "  Timestep: " << config.getTimeStep() << " s\n";
    std::cout << "  Max iterations: " << config.getMaxIterations() << "\n";
    std::cout << "  Config load time: " << configTimer.elapsedMilliseconds() << " ms\n";
    std::cout << "----------------------------------------\n";

    PhysicsWorld world(config);
    world.initialize();

    // Input loop
    while (true)
    {
        char* raw = linenoise("> ");
        if (!raw)
            break;

        std::string command(raw);
        linenoiseFree(raw);

        if (command.empty())
            continue;

        // parse words after getting command
        auto words = parseWords(command);
        if (words.empty())
        {
            printUsage();
            continue;
        }

        // Add command to hystory
        linenoiseHistoryAdd(command.c_str());
        linenoiseHistorySave(historyFilename);

        const std::string actionStr = popNext(words);
        CommandType       action    = commandFromString(actionStr);

        // Indicator to know if the command was executed successfully
        bool success = false;

        switch (action)
        {
        case CommandType::HELP:
            printUsage();
            success = true;
            break;

        case CommandType::EXIT:
            world.clearObjects();
            trimHistory(historyFilename, 1000);
            std::cout << "Exiting simulation.\n";
            success = true;
            return 0;

        case CommandType::START:
            world.start();
            std::cout << "Simulation started.\n";
            success = true;
            break;

        case CommandType::STOP:
            world.stop();
            std::cout << "Simulation stopped.\n";
            success = true;
            break;

        case CommandType::RUN:
            if (!world.getIsRunning())
                std::cout << "Simulation is not running. Run start first.\n";
            world.run();
            success = true;
            break;

        case CommandType::INTEGRATE:
            if (!words.empty())
            {
                const decimal dt = stringToDecimal(popNext(words));
                world.integrate();
                std::cout << "Integrated one step of " << dt << "s.\n";
                success = true;
            }
            else
            {
                std::cout << "Usage: integrate <dt>\n";
            }
            break;

        case CommandType::PRINT:
            world.printState();
            success = true;
            break;

        case CommandType::INIT:
            world.initialize();
            std::cout << "World initialized.\n";
            success = true;
            break;

        case CommandType::SET:
            success = handleSetCommand(world, words);
            break;

        case CommandType::ADD: {
            success = handleAddCommand(world, words);
            break;
        }

        case CommandType::LIST:
            listObjects(world);
            success = true;
            break;

        case CommandType::SHOW:
            if (!words.empty())
            {
                size_t id = std::stoul(popNext(words));
                showObject(world, id);
                success = true;
            }
            else
            {
                std::cout << "Usage: show <id>\n";
            }
            break;

        case CommandType::DEL:
            if (!words.empty())
            {
                size_t id = std::stoul(popNext(words));
                world.removeObject(world.getObject(id));
                std::cout << "Removed object " << id << "\n";
                success = true;
            }
            else
            {
                std::cout << "Usage: del <id>\n";
            }
            break;

        default:
            std::cout << "Unknown command: " << actionStr << "\n";
            printUsage();
            break;
        }

        // Record successful command in history and completion (encapsulated)
        if (success)
            recordSuccessfulCommand(historyFilename, command);
    }

    return 0;
}

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

void clearFile(const std::string& filename, std::size_t maxLines)
{
    std::cout << "Clearing history file..." << std::endl;
    std::ifstream in(filename);
    if (!in.is_open())
    {
        std::cout << "Incorrect history filename !" << std::endl;
        return;
    }

    std::vector<std::string> lines;
    std::string              line;

    // Load all lines
    while (std::getline(in, line))
    {
        lines.push_back(std::move(line));
    }
    in.close();

    if (lines.size() <= maxLines)
    {
        std::cout << "File lines are below threshold, nothing to clear." << std::endl;
        return;
    }

    // Keep only the last maxLines lines
    std::vector<std::string> trimmed(lines.end() - maxLines, lines.end());

    // Rewrite the file
    std::ofstream out(filename, std::ios::trunc);
    if (!out.is_open())
    {
        return;
    }

    for (const auto& s : trimmed)
    {
        out << s << '\n';
    }
    std::cout << "History file cleared !" << std::endl;
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
// Completion callback (linenoise)
// ============================================================================
void completionAdd(const std::string& filename, const std::string& command)
{
    // Use a set to avoid duplicates
    std::unordered_set<std::string> commands;

    // Read existing commands
    std::ifstream in(filename);
    if (in.is_open())
    {
        std::string line;
        while (std::getline(in, line))
        {
            if (!line.empty())
                commands.insert(line);
        }
        in.close();
    }
    else
    {
        std::cout << "Incorrect completion filename: " << filename << std::endl;
    }

    // Insert the new command if it does not already exist
    if (!command.empty() && commands.find(command) == commands.end())
    {
        std::ofstream out(filename, std::ios::app);
        if (out.is_open())
        {
            out << command << '\n';
        }
        else
        {
            std::cerr << "Failed to open completion file for writing: " << filename << std::endl;
        }
    }
}

void completionCallback(const char* prefix, linenoiseCompletions* lc)
{
    std::vector<std::string> allCommands;
    allCommands.reserve(100);

    // Load commands from file
    {
        std::ifstream in(completionFilename);
        std::string   line;
        if (in.is_open())
        {
            while (std::getline(in, line))
            {
                if (!line.empty())
                    allCommands.push_back(line);
            }
        }
    }

    // Perform autocompletion
    const size_t pLen = std::strlen(prefix);

    for (const auto& cmd : allCommands)
    {
        if (cmd.size() >= pLen && std::strncmp(prefix, cmd.c_str(), pLen) == 0)
        {
            linenoiseAddCompletion(lc, cmd.c_str());
        }
    }
}

// ============================================================================
// Main
// ============================================================================
int main(int argc, char** argv)
{
    linenoiseSetCompletionCallback(completionCallback);
    linenoiseHistoryLoad(historyFilename);

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

    using PropertySetter = std::function<void(Object*, const std::vector<std::string>& args)>;
    static const std::unordered_map<std::string, PropertySetter> PROPERTY_SETTERS = {
        { "pos", [](Object* obj, const auto& a)
          { obj->setPosition({ stringToDecimal(a[0]), stringToDecimal(a[1]), stringToDecimal(a[2]) }); } },
        { "vel", [](Object* obj, const auto& a)
          { obj->setVelocity({ stringToDecimal(a[0]), stringToDecimal(a[1]), stringToDecimal(a[2]) }); } },
        { "acc",
          [](Object* obj, const auto& a)
          {
              obj->setAcceleration({ stringToDecimal(a[0]), stringToDecimal(a[1]), stringToDecimal(a[2]) });
          } },
        { "rot", [](Object* obj, const auto& a)
          { obj->setRotation({ stringToDecimal(a[0]), stringToDecimal(a[1]), stringToDecimal(a[2]) }); } },
        { "mass", [](Object* obj, const auto& a) { obj->setMass(stringToDecimal(a[0])); } },
        { "fixed",
          [](Object* obj, const auto& a)
          {
              bool                                               b                = (a[0] == "1" || a[0] == "true" || a[0] == "yes");
              obj->setIsFixed(b);
          } },
    };

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

        linenoiseHistoryAdd(command.c_str());
        linenoiseHistorySave(historyFilename);

        auto words = parseWords(command);
        if (words.empty())
        {
            printUsage();
            continue;
        }

        const std::string actionStr = popNext(words);
        CommandType       action    = commandFromString(actionStr);

        // Indicateur pour savoir si la commande a été exécutée correctement
        bool success = false;

        switch (action)
        {
        case CommandType::HELP:
            printUsage();
            success = true;
            break;

        case CommandType::EXIT:
            world.clearObjects();
            clearFile(std::string(1, *historyFilename), 1000);
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
                world.integrate(dt);
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
            if (!words.empty())
            {
                const std::string what = popNext(words);
                if (what == "dt" && !words.empty())
                {
                    const decimal value = stringToDecimal(popNext(words));
                    world.setTimeStep(value);
                    std::cout << "Timestep set to " << value << "s.\n";
                    success = true;
                }
                else if (what == "g" && !words.empty())
                {
                    const decimal value = stringToDecimal(popNext(words));
                    world.setGravityCst(value);
                    world.setGravityAcc(Physics::computeGravityAcc(value));
                    std::cout << "Gravity set to " << value << " m/s².\n";
                    success = true;
                }
                else if (what == "obj" && words.size() >= 2)
                {
                    size_t      id   = std::stoul(popNext(words));
                    std::string prop = popNext(words);

                    Object* obj = world.getObject(id);
                    if (obj && PROPERTY_SETTERS.count(prop))
                    {
                        std::vector<std::string> args(words.begin(), words.end());
                        PROPERTY_SETTERS.at(prop)(obj, args);
                        std::cout << "Set " << prop << " on object " << id << "\n";
                        success = true;
                    }
                    else
                    {
                        std::cout << "Invalid object id or property.\n";
                    }
                }
            }
            break;

        case CommandType::ADD: {
            if (words.empty())
            {
                std::cout << "Usage: add <sphere|plane|AABB>\n";
                break;
            }
            const std::string type = popNext(words);
            if (type == "sphere")
            {
                auto sphere = std::make_unique<Sphere>(Vector3D(0, 0, 10), 0.2_d, Vector3D(0, 0, 0), 1.0_d);
                world.addObject(sphere.release());
                std::cout << "Added sphere.\n";
                success = true;
            }
            else if (type == "plane")
            {
                auto plane =
                    std::make_unique<Plane>(Vector3D(0, 0, 0), Vector3D(1, 1, 1), 1.0_d, Vector3D(0, 1, 0));
                world.addObject(plane.release());
                std::cout << "Added plane.\n";
                success = true;
            }
            else if (type == "AABB")
            {
                auto aabb =
                    std::make_unique<AABB>(Vector3D(0, 0, 5), Vector3D(1, 1, 1), Vector3D(0, 0, 0), 1.0_d);
                world.addObject(aabb.release());
                std::cout << "Added AABB.\n";
                success = true;
            }
            else
            {
                std::cout << "Unknown object type: " << type << "\n";
            }
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

        // Ajouter au fichier de complétion **seulement si la commande a été exécutée avec succès**
        if (success)
            completionAdd(completionFilename, command);
    }

    return 0;
}

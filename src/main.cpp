#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "utilities/command.hpp"
#include "utilities/timer.hpp"
#include "world/config.hpp"
#include "world/physicsWorld.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <utility>

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
                      << " | vel=" << obj->getVelocity() << (obj->isFixed() ? " | fixed" : "") << "\n";
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
// Main entry point
// ============================================================================
int main(int argc, char** argv)
{
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

    std::cout << "Enter 'help' to see available commands.\n";

    while (true)
    {
        std::cout << "\n> ";
        std::string command;
        std::getline(std::cin, command);
        if (command.empty())
            continue;

        auto words = parseWords(command);
        if (words.empty())
        {
            printUsage();
            continue;
        }

        const std::string action = popNext(words);

        // === COMMANDS ===
        if (action == "help" || action == "h")
        {
            printUsage();
        }
        else if (action == "exit" || action == "quit")
        {
            break;
        }
        else if (action == "init")
        {
            world.initialize();
            std::cout << "World initialized.\n";
        }
        else if (action == "start")
        {
            world.start();
            std::cout << "Simulation started.\n";
        }
        else if (action == "stop")
        {
            world.stop();
            std::cout << "Simulation stopped.\n";
        }
        else if (action == "step")
        {
            if (words.empty())
            {
                std::cout << "Usage: step <dt>\n";
                continue;
            }
            const decimal dt = std::stod(popNext(words));
            world.integrate(dt);
            std::cout << "Integrated one step of " << dt << "s.\n";
        }
        else if (action == "set")
        {
            if (words.empty())
            {
                std::cout << "Usage: set <dt|g|obj>\n";
                continue;
            }
            const std::string what = popNext(words);

            if (what == "dt")
            {
                const decimal value = std::stod(popNext(words));
                world.setTimeStep(value);
                std::cout << "Timestep set to " << value << "s.\n";
            }
            else if (what == "g")
            {
                const decimal value = std::stod(popNext(words));
                world.setGravityCst(value);
                world.setGravityAcc(Physics::computeGravityAcc(value));
                std::cout << "Gravity set to " << value << " m/s².\n";
            }
            else if (what == "obj")
            {
                if (words.size() < 5)
                {
                    std::cout << "Usage: set obj <id> <pos|vel> <x> <y> <z>\n";
                    continue;
                }

                size_t      id   = std::stoul(popNext(words));
                std::string prop = popNext(words);
                decimal     x    = std::stod(popNext(words));
                decimal     y    = std::stod(popNext(words));
                decimal     z    = std::stod(popNext(words));

                Object* obj = world.getObject(id);
                if (!obj)
                {
                    std::cout << "No object with id " << id << "\n";
                    continue;
                }

                if (prop == "pos")
                {
                    obj->setPosition(Vector3D(x, y, z));
                    std::cout << "Set position of object " << id << " to (" << x << ", " << y << ", " << z
                              << ")\n";
                }
                else if (prop == "vel")
                {
                    obj->setVelocity(Vector3D(x, y, z));
                    std::cout << "Set velocity of object " << id << " to (" << x << ", " << y << ", " << z
                              << ")\n";
                }
                else
                {
                    std::cout << "Unknown property: " << prop << "\n";
                }
            }
        }
        else if (action == "add")
        {
            if (words.empty())
            {
                std::cout << "Usage: add <sphere|plane|AABB>\n";
                continue;
            }

            const std::string type = popNext(words);
            if (type == "sphere")
            {
                auto sphere = std::make_unique<Sphere>(Vector3D(0, 0, 10), // position
                                                       0.2_d,              // size (radius)
                                                       Vector3D(0, 0, 0),  // velocity
                                                       1.0_d               // mass
                );
                world.addObject(sphere.release());
                std::cout << "Added sphere with default parameters.\n";
            }
            else if (type == "plane")
            {
                auto plane = std::make_unique<Plane>(Vector3D(0, 0, 0), // position
                                                     Vector3D(1, 1, 1), // size
                                                     1.0_d,             // mass
                                                     Vector3D(0, 1, 0)  // normal
                );
                world.addObject(plane.release());
                std::cout << "Added plane with default parameters.\n";
            }
            else if (type == "AABB")
            {
                auto aabb = std::make_unique<AABB>(Vector3D(0, 0, 5), // position
                                                   Vector3D(1, 1, 1), // size
                                                   Vector3D(0, 0, 0), // velocity
                                                   1.0_d              // mass
                );
                world.addObject(aabb.release());
                std::cout << "Added AABB with default parameters.\n";
            }
            else
            {
                std::cout << "Unknown object type: " << type << "\n";
            }
        }
        else if (action == "list")
        {
            listObjects(world);
        }
        else if (action == "show")
        {
            if (words.empty())
            {
                std::cout << "Usage: show <id>\n";
                continue;
            }
            size_t id = std::stoul(popNext(words));
            showObject(world, id);
        }
        else if (action == "del")
        {
            if (words.empty())
            {
                std::cout << "Usage: del <id>\n";
                continue;
            }
            size_t id = std::stoul(popNext(words));
            world.removeObject(world.getObject(id));
            std::cout << "Removed object " << id << "\n";
        }
        else if (action == "print")
        {
            world.printState();
        }
        else
        {
            std::cout << "Unknown command: " << action << "\n";
            printUsage();
        }
    }

    std::cout << "Exiting simulation.\n";
    return 0;
}

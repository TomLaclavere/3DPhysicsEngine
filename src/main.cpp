#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"
#include "precision.hpp"
#include "utilities/command.hpp"
#include "utilities/timer.hpp"
#include "world/config.hpp"
#include "world/physicsWorld.hpp"

#include <chrono>
#include <functional>
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

    // Initialise world
    PhysicsWorld world(config);
    world.initialize();

    // Define a property function type
    using PropertySetter = std::function<void(Object*, const std::vector<std::string>& args)>;
    static const std::unordered_map<std::string, PropertySetter> PROPERTY_SETTERS = {
        // Vector properties (3 components)
        { "pos", [](Object* obj, const auto& a)
          { obj->setPosition({ stringToDecimal(a[0]), stringToDecimal(a[1]), stringToDecimal(a[2]) }); } },
        { "vel", [](Object* obj, const auto& a)
          { obj->setVelocity({ stringToDecimal(a[0]), stringToDecimal(a[1]), stringToDecimal(a[2]) }); } },
        { "acc",
          [](Object* obj, const auto& a) {
              obj->setAcceleration({ stringToDecimal(a[0]), stringToDecimal(a[1]), stringToDecimal(a[2]) });
          } },
        { "rot", [](Object* obj, const auto& a)
          { obj->setRotation({ stringToDecimal(a[0]), stringToDecimal(a[1]), stringToDecimal(a[2]) }); } },

        // Scalar properties
        { "mass", [](Object* obj, const auto& a) { obj->setMass(stringToDecimal(a[0])); } },

        // Boolean properties
        { "fixed",
          [](Object* obj, const auto& a)
          {
              bool                                               b                = (a[0] == "1" || a[0] == "true" || a[0] == "yes");
              obj->setIsFixed(b);
          } },
    };

    // Start loop
    std::cout << "Enter command:" << std::endl;
    std::cout << "(Enter 'help' to see available commands.)" << std::endl;
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
            world.clearObjects();
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
        else if (action == "run")
        {
            world.run();
        }
        else if (action == "integrate")
        {
            if (words.empty())
            {
                std::cout << "Usage: integrate <dt>\n";
                continue;
            }
            const decimal dt = stringToDecimal(popNext(words));
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
                const decimal value = stringToDecimal(popNext(words));
                world.setTimeStep(value);
                std::cout << "Timestep set to " << value << "s.\n";
            }
            else if (what == "g")
            {
                const decimal value = stringToDecimal(popNext(words));
                world.setGravityCst(value);
                world.setGravityAcc(Physics::computeGravityAcc(value));
                std::cout << "Gravity set to " << value << " m/s².\n";
            }
            else if (what == "obj")
            {
                if (words.size() < 2)
                {
                    std::cout << "Usage: set obj <id> <property> [...values]\n";
                    continue;
                }

                size_t      id   = std::stoul(popNext(words));
                std::string prop = popNext(words);

                Object* obj = world.getObject(id);
                if (!obj)
                {
                    std::cout << "No object with id " << id << "\n";
                    continue;
                }

                // Check if property exists
                auto it = PROPERTY_SETTERS.find(prop);
                if (it == PROPERTY_SETTERS.end())
                {
                    std::cout << "Unknown property: " << prop << "\n";
                    continue;
                }

                // Remaining parameters = arguments for the setter
                std::vector<std::string> args(words.begin(), words.end());
                it->second(obj, args);

                std::cout << "Set " << prop << " on object " << id << "\n";
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

#include "utilities/command.hpp"

#include "objects/aabb.hpp"
#include "objects/plane.hpp"
#include "objects/sphere.hpp"

#include <functional>
#include <iostream>
#include <memory>
#include <sstream>

// ============================================================================
// Commands string utilities
// ============================================================================
void printUsage()
{
    std::cout
        << "Available commands:\n"
        << "-------------------------------------------------------------------------------------\n"
        << "Commands:\n"
        << "  help / h                             Show this help message.\n"
        << "  exit / quit / q                      Quit program.\n"
        << "-------------------------------------------------------------------------------------\n"
        << "Simulation:\n"
        << "  start                                Start simulation.\n"
        << "  stop                                 Stop simulation.\n"
        << "  run                                  Run the simulation with set parameters.\n"
        << "  integrate <dt>                       Integrate one timestep.\n"
        << "  print                                Print world summary.\n"
        << "  init                                 (Re-)Initialize world.\n"
        << "-------------------------------------------------------------------------------------\n"
        << "World:\n"
        << "  set <dt|g> <value>                   Set timestep/grav acceleration to <value>.\n"
        << "  set obj <id> <property> [...values]  Set property for a given object, identified by its ID "
           "(see 'list'). Properties can be: pos, vel, acc, rot, mass, fixed. values can "
           "be either one value (ex: mass) or three values separated by spaces (ex: position).\n"
        << "  add <sphere|plane|AABB>              Add an object to the world.\n"
        << "  list                                 List all objects in the format <id> <ObjectType> "
           "<position> <velocity> <fixed>.\n"
        << "  show <id>                            Show a specific object.\n"
        << "  del <id>                             Delete an object by ID.\n"
        << "-------------------------------------------------------------------------------------\n";
}

std::deque<std::string> parseWords(const std::string& command)
{
    auto words = std::deque<std::string> {};

    auto str = std::stringstream { command };
    while (str.good())
    {
        str >> words.emplace_back();
    }

    return words;
}

std::string popNext(std::deque<std::string>& words)
{
    if (words.empty())
    {
        return "";
    }
    else
    {
        auto next = std::move(words.front());
        words.pop_front();
        return next;
    }
}

bool isValidName(const std::string& name)
{
    return !name.empty() && std::all_of(name.begin(), name.end(), [](char c) { return std::isalnum(c); });
}

std::vector<std::string> consumeNames(std::deque<std::string>& words)
{
    auto names = std::vector<std::string> {};

    while (!words.empty())
    {
        if (!isValidName(words.front()))
        {
            break;
        }

        names.emplace_back(popNext(words));
    }

    return names;
}

// ============================================================================
// Commands handler
// ============================================================================
/// @brief Map property name: setter callback used by handleSetCommand.
using PropertySetter = std::function<void(Object*, const std::vector<std::string>&)>;

static const std::unordered_map<std::string, PropertySetter> PROPERTY_SETTERS = {
    { "pos", [](Object* o, const auto& a) { setVector3Property(o, a, &Object::setPosition, "pos"); } },
    { "vel", [](Object* o, const auto& a) { setVector3Property(o, a, &Object::setVelocity, "vel"); } },
    { "acc", [](Object* o, const auto& a) { setVector3Property(o, a, &Object::setAcceleration, "acc"); } },
    { "rot", [](Object* o, const auto& a) { setVector3Property(o, a, &Object::setRotation, "rot"); } },
    { "mass", [](Object* o, const auto& a) { setScalarProperty(o, a, &Object::setMass, "mass"); } },
    { "fixed", [](Object* o, const auto& a) { setFixedProperty(o, a); } },
};

bool handleSetCommand(PhysicsWorld& world, std::deque<std::string>& words)
{
    if (words.empty())
        return false;

    const std::string what = popNext(words);

    if (what == "dt" && !words.empty())
    {
        const decimal value = stringToDecimal(popNext(words));
        world.setTimeStep(value);
        std::cout << "Timestep set to " << value << "s.\n";
        return true;
    }
    if (what == "g" && !words.empty())
    {
        const decimal value = stringToDecimal(popNext(words));
        world.setGravityCst(value);
        world.setGravityAcc(Physics::computeGravityAcc(value));
        std::cout << "Gravity set to " << value << " m/s².\n";
        return true;
    }
    if (what == "obj" && words.size() >= 2)
    {
        size_t      id   = std::stoul(popNext(words));
        std::string prop = popNext(words);

        Object* obj = world.getObject(id);
        if (obj && PROPERTY_SETTERS.count(prop))
        {
            std::vector<std::string> args(words.begin(), words.end());
            PROPERTY_SETTERS.at(prop)(obj, args);
            std::cout << "Set " << prop << " on object " << id << "\n";
            return true;
        }
        std::cout << "Invalid object id or property.\n";
    }
    return false;
}

bool handleAddCommand(PhysicsWorld& world, std::deque<std::string>& words)
{
    if (words.empty())
    {
        std::cout << "Usage: add <sphere|plane|AABB>\n";
        return false;
    }

    const std::string type = popNext(words);
    if (type == "sphere")
    {
        auto sphere = std::make_unique<Sphere>(Vector3D(0, 0, 10), 0.2_d, Vector3D(0, 0, 0), 1.0_d);
        world.addObject(sphere.release());
        std::cout << "Added sphere.\n";
        return true;
    }
    if (type == "plane")
    {
        auto plane = std::make_unique<Plane>(Vector3D(0, 0, 0), Vector3D(1, 1, 1), 1.0_d, Vector3D(0, 1, 0));
        world.addObject(plane.release());
        std::cout << "Added plane.\n";
        return true;
    }
    if (type == "AABB")
    {
        auto aabb = std::make_unique<AABB>(Vector3D(0, 0, 5), Vector3D(1, 1, 1), Vector3D(0, 0, 0), 1.0_d);
        world.addObject(aabb.release());
        std::cout << "Added AABB.\n";
        return true;
    }

    std::cout << "Unknown object type: " << type << "\n";
    return false;
}

// ============================================================================
// Arguments parser
// ============================================================================
void setVector3Property(Object* obj, const std::vector<std::string>&         args,
                        void (Object::*setter)(const Vector3D&), const char* name)
{
    if (args.size() < 3)
    {
        std::cout << "Usage: set obj <id> " << name << " <x> <y> <z>\n";
        return;
    }
    const Vector3D v { stringToDecimal(args[0]), stringToDecimal(args[1]), stringToDecimal(args[2]) };
    (obj->*setter)(v);
}

void setScalarProperty(Object* obj, const std::vector<std::string>& args, void (Object::*setter)(decimal),
                       const char* name)
{
    if (args.empty())
    {
        std::cout << "Usage: set obj <id> " << name << " <value>\n";
        return;
    }
    (obj->*setter)(stringToDecimal(args[0]));
}

void setFixedProperty(Object* obj, const std::vector<std::string>& args)
{
    if (args.empty())
    {
        std::cout << "Usage: set obj <id> fixed <0|1|true|false|yes|no>\n";
        return;
    }
    const std::string& val = args[0];
    const bool         b   = (val == "1" || val == "true" || val == "yes");
    obj->setIsFixed(b);
}

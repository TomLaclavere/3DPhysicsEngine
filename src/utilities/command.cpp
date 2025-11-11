#include "utilities/command.hpp"

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

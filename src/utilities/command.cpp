#include "utilities/command.hpp"

void printUsage()
{
    std::cout << "Available commands:\n"
              << "  help                     Show this help message\n"
              << "  init                     Initialize world\n"
              << "  start                    Start simulation\n"
              << "  stop                     Stop simulation\n"
              << "  step <dt>                Integrate one timestep\n"
              << "  set dt <value>           Set timestep\n"
              << "  set g <value>            Set gravity constant\n"
              << "  add <sphere|plane|cube>  Add an object to the world\n"
              << "  set obj <id> <property> <x> <y> <z>  Set object position or velocity\n"
              << "  list                     List all objects\n"
              << "  show <id>                Show a specific object\n"
              << "  del <id>                 Delete an object by ID\n"
              << "  print                    Print world summary\n"
              << "  exit                     Quit program\n";
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

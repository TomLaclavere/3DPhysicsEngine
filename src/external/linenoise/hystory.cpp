#include "external/linenoise/linenoise.h"
#include "utilities/command.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <vector>

static std::string g_completionFilename;

static void completionAdd(const std::string& filename, const std::string& command)
{
    std::unordered_set<std::string> commands;
    std::ifstream                   in(filename);
    if (in.is_open())
    {
        std::string line;
        while (std::getline(in, line))
            if (!line.empty())
                commands.insert(line);
    }

    if (!command.empty() && !commands.count(command))
    {
        std::ofstream out(filename, std::ios::app);
        if (out.is_open())
            out << command << '\n';
    }
}

static void completionCallback(const char* prefix, linenoiseCompletions* lc)
{
    std::vector<std::string> allCommands;
    std::ifstream            in(g_completionFilename);
    std::string              line;
    if (in.is_open())
    {
        while (std::getline(in, line))
            if (!line.empty())
                allCommands.push_back(line);
    }

    const size_t pLen = std::strlen(prefix);
    for (const auto& cmd : allCommands)
        if (cmd.size() >= pLen && std::strncmp(prefix, cmd.c_str(), pLen) == 0)
            linenoiseAddCompletion(lc, cmd.c_str());
}

void initHistoryAndCompletion(const char* historyFilename, const std::string& completionFilename)
{
    g_completionFilename = completionFilename;
    linenoiseSetCompletionCallback(completionCallback);
    linenoiseHistoryLoad(historyFilename);
}

void recordSuccessfulCommand(const char* historyFilename, const std::string& command)
{
    completionAdd(g_completionFilename, command);
}

void trimHistory(const char* historyFilename, std::size_t maxLines)
{
    std::cout << "Clearing history file..." << std::endl;
    std::ifstream in(historyFilename);
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
    std::ofstream out(historyFilename, std::ios::trunc);
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

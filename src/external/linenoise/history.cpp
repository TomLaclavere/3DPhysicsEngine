#include "external/linenoise/linenoise.h"
#include "utilities/command.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <vector>

/// @brief Path to the file storing known completion commands.
static std::string g_completionFilename;

/**
 * @brief Add a command to the completion file if it does not already exist.
 *
 * The file is treated as a set of unique commands (one per line).
 * The command is appended only if it is non-empty and not already present.
 *
 * @param filename Path to the completion file.
 * @param command  Command to record for future autocompletion.
 */
static void completionAdd(const std::string& filename, const std::string& command)
{
    std::unordered_set<std::string> commands;
    std::ifstream                   in(filename);

    // Load existing commands
    if (in.is_open())
    {
        std::string line;
        while (std::getline(in, line))
            if (!line.empty())
                commands.insert(line);
    }

    // Append command if new
    if (!command.empty() && !commands.count(command))
    {
        std::ofstream out(filename, std::ios::app);
        if (out.is_open())
            out << command << '\n';
    }
}

/**
 * @brief linenoise completion callback.
 *
 * This function is called by linenoise whenever the user presses the
 * completion key (usually TAB). It loads all known commands from the
 * completion file and suggests those matching the given prefix.
 *
 * @param prefix The current input buffer.
 * @param lc     linenoise completion list to populate.
 */
static void completionCallback(const char* prefix, linenoiseCompletions* lc)
{
    std::vector<std::string> allCommands;
    std::ifstream            in(g_completionFilename);
    std::string              line;

    // Load all known completion commands
    if (in.is_open())
    {
        while (std::getline(in, line))
            if (!line.empty())
                allCommands.push_back(line);
    }

    // Match prefix
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

void recordSuccessfulCommand(const std::string& command) { completionAdd(g_completionFilename, command); }

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

    // Nothing to do if under threshold
    if (lines.size() <= maxLines)
    {
        std::cout << "File lines are below threshold, nothing to clear." << std::endl;
        return;
    }

    // Keep only the last maxLines lines
    using diff_t = std::vector<std::string>::difference_type;
    std::vector<std::string> trimmed(lines.end() - static_cast<diff_t>(maxLines), lines.end());

    // Rewrite history file
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

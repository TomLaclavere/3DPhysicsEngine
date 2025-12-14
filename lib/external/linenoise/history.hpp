/**
 * @file hystory.hpp
 * @brief Command history and autocompletion utilities based on linenoise.
 *
 * This module provides:
 * - Persistent command history
 * - File-backed command autocompletion
 * - History trimming utilities
 *
 * It relies on the linenoise library for interactive input handling.
 * @see linenoise.h
 */
#pragma once
#include "external/linenoise/linenoise.h"

#include <string>

/**
 * @brief Initialize history loading and command autocompletion.
 *
 * This function:
 *  - Sets the global completion filename
 *  - Registers the completion callback with linenoise
 *  - Loads command history from file
 *
 * @param historyFilename     Path to the history file.
 * @param completionFilename  Path to the completion file.
 */
void initHistoryAndCompletion(const char* historyFilename, const std::string& completionFilename);

/**
 * @brief Record a successfully executed command.
 *
 * The command is added to the completion file so it becomes available
 * for future autocompletion.
 *
 * @param historyFilename Unused (kept for API consistency).
 * @param command         Command that was successfully executed.
 */
void recordSuccessfulCommand(const std::string& command);

/**
 * @brief Trim the command history file to a maximum number of lines.
 *
 * If the history file exceeds @p maxLines, only the most recent entries
 * are preserved. The file is rewritten in place.
 *
 * @param historyFilename Path to the history file.
 * @param maxLines        Maximum number of history entries to keep.
 */
void trimHistory(const char* historyFilename, std::size_t maxLines);

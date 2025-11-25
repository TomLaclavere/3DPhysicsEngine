#pragma once
#include "external/linenoise/linenoise.h"

#include <string>

void initHistoryAndCompletion(const char* historyFilename, const std::string& completionFilename);

void recordSuccessfulCommand(const char* historyFilename, const std::string& command);

void trimHistory(const char* historyFilename, std::size_t maxLines);

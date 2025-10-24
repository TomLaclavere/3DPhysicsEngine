#include "precision.hpp"

#include <algorithm>
#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

void printUsage();

std::deque<std::string> parseWords(const std::string& command);

std::string popNext(std::deque<std::string>& words);

bool isValidName(const std::string& name);

std::vector<std::string> consumeNames(std::deque<std::string>& words);

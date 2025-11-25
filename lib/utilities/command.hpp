#pragma once
#include "objects/object.hpp"
#include "precision.hpp"
#include "world/physicsWorld.hpp"

#include <algorithm>
#include <deque>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

void printUsage();

std::deque<std::string> parseWords(const std::string& command);

std::string popNext(std::deque<std::string>& words);

bool isValidName(const std::string& name);

std::vector<std::string> consumeNames(std::deque<std::string>& words);

// Command handler functions
bool handleSetCommand(PhysicsWorld& world, std::deque<std::string>& words);
bool handleAddCommand(PhysicsWorld& world, std::deque<std::string>& words);

// Property setter functions
void setVector3Property(Object* obj, const std::vector<std::string>&         args,
                        void (Object::*setter)(const Vector3D&), const char* name);
void setScalarProperty(Object* obj, const std::vector<std::string>& args, void (Object::*setter)(decimal),
                       const char* name);
void setFixedProperty(Object* obj, const std::vector<std::string>& args);

// Utility functions
void        printUsage();
std::string popNext(std::deque<std::string>& words);

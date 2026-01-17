/**
 * @file command.hpp
 * @brief Wrapper for utility functions used for commands in main.
 */
#pragma once
#include "objects/object.hpp"
#include "world/physicsWorld.hpp"

#include <deque>
#include <string>
#include <vector>

// ============================================================================
/// @name Commands string utilities
// ============================================================================
/// @{

/// Print CLI help and usage text for commands.
void printUsage();

/// @brief Tokenise an input command line into whitespace-separated words.
/// @param command Input line to parse
/// @return Deque of tokens in original order.
std::deque<std::string> parseWords(const std::string& command);

/// @brief Consume and return next token from a deque.
/// @param words Mutable deque of tokens
/// @return Next token
std::string popNext(std::deque<std::string>& words);

/// @brief Validate that a token is a simple alphanumeric "name".
/// @param name Token to validate.
/// @return True if non-empty and all characters are alphanumeric, false otherwise.
bool isValidName(const std::string& name);

/// @brief Consume a contiguous prefix of valid names from deque.
/// @param words Deque of tokens
/// @return Vector of consumed name tokens.
std::vector<std::string> consumeNames(std::deque<std::string>& words);
/// @}

// ============================================================================
/// @name Commands handler
// ============================================================================
/// @{

/// @brief Execute the top-level "set" command, to set simulation parameter's value.
/// @param world PhysicsWorld instance.
/// @param words Deque of remaining tokens after "set".
/// @return True on success (change applies), false on failure/invalid usage.
bool handleSetCommand(PhysicsWorld& world, std::deque<std::string>& words);

/// @brief Execute the top-level "ADD" command, to add an object to the simulation.
/// @param world PhysicsWorld instance.
/// @param words Deque with object type token.
/// @return True on success (object added), false on failure/invalid usage.
bool handleAddCommand(PhysicsWorld& world, std::deque<std::string>& words);
/// @}

// ============================================================================
/// @name Arguments parser
// ============================================================================
/// @{

/// @brief Parse three numeric args and call an Object Vector3 setter.
/// @param obj Object instance pointer.
/// @param args Expected 3 string tokens.
/// @param setter Pointer-to-member function on Object to call.
/// @param name Property name (used in usage message).
void setVector3Property(Object* obj, const std::vector<std::string>&         args,
                        void (Object::*setter)(const Vector3D&), const char* name);
/// @brief Parse single numeric arg and call Object scalar setter.
/// @param obj Object instance pointer.
/// @param args Expected 1 string tokens.
/// @param setter Pointer-to-member function on Object to call.
/// @param name Property name (used in usage message).
void setScalarProperty(Object* obj, const std::vector<std::string>& args, void (Object::*setter)(decimal),
                       const char* name);
/// @brief Parse boolean-like token and set Object::IsFixed.
/// @param obj Object instance pointer.
/// @param args Expected 1 token ("1"/"0"/"true"/"false"/"yes"/"no").
void setFixedProperty(Object* obj, const std::vector<std::string>& args);
/// @}

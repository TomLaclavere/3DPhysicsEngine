/**
 * @file collision_response.hpp
 * @brief Declaration of collision response functions between physics objects.
 *
 * Provides functions to compute the back force applied on object in collision.
 *
 * @note Be careful, this file does not include angular correction for now.
 */

#pragma once

#include "collision.hpp"

void positionCorrection(Object& A, Object& B);

void reboundCollision(Object& A, Object& B);

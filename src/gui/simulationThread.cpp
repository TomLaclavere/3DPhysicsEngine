/**
 * @file simulationThread.cpp
 * @brief Implementation of SimulationThread: runs the physics world in a background QThread.
 */
#include "gui/simulationThread.hpp"

SimulationThread::SimulationThread(PhysicsWorld& world, QObject* parent)
    : QThread(parent)
    , m_world(world)
{}

void SimulationThread::run()
{
    m_world.run();
    emit simulationFinished();
}

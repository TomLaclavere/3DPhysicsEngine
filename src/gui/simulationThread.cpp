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

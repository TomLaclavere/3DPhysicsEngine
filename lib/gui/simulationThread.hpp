/**
 * @file simulationThread.hpp
 * @brief QThread subclass that runs the physics simulation loop off the UI thread.
 */
#pragma once
#include "world/physicsWorld.hpp"

#include <QThread>

/**
 * @class SimulationThread
 * @brief Runs @ref PhysicsWorld::run() in a dedicated thread.
 *
 * Emits @ref simulationFinished when the run completes so the main window
 * can re-enable controls.
 */
class SimulationThread : public QThread
{
    Q_OBJECT
public:
    explicit SimulationThread(PhysicsWorld& world, QObject* parent = nullptr);
    void run() override;

signals:
    void simulationFinished();

private:
    PhysicsWorld& m_world;
};
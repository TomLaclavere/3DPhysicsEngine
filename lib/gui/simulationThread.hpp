#pragma once
#include "world/physicsWorld.hpp"

#include <QThread>

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
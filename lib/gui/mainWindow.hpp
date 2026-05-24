#pragma once
#include "gui/simulationThread.hpp"
#include "world/config.hpp"
#include "world/physicsWorld.hpp"

#include <QMainWindow>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onInit();
    void onStart();
    void onStop();
    void onRun();
    void onPrint();
    void onStep();
    void onApplyConfig();
    void onAddObject();
    void onObjectTableClicked(int row, int col);
    void onSetObject();
    void onDelObject();
    void onPlotTrajectories();
    void onPlotObjects();
    void onPlotAnimation();
    void onSimulationFinished();

private:
    void buildUi();
    void refreshObjectTable();
    void setControlsEnabled(bool enabled);
    void log(const QString& msg);
    void runPlotScript(const QString& mode);

    Config&          m_config;
    PhysicsWorld     m_world;
    SimulationThread m_thread;

    // Config widgets
    QDoubleSpinBox* m_gravityBox {};
    QDoubleSpinBox* m_dtBox {};
    QDoubleSpinBox* m_durationBox {};
    QComboBox*      m_solverBox {};

    QCheckBox* m_simplifiedCollisionBox {};

    // Config default material widgets
    QDoubleSpinBox* m_cfgStiffnessBox {};
    QDoubleSpinBox* m_cfgDampingBox {};
    QDoubleSpinBox* m_cfgFrictionBox {};
    QDoubleSpinBox* m_cfgRestitutionBox {};

    // Add object widgets
    QComboBox* m_typeBox {};
    QLineEdit* m_nameEdit {};

    // Object table
    QTableWidget* m_objectTable {};

    // Selected object properties
    QLabel*         m_selectedLabel {};
    QDoubleSpinBox* m_posX {};
    QDoubleSpinBox* m_posY {};
    QDoubleSpinBox* m_posZ {};
    QDoubleSpinBox* m_velX {};
    QDoubleSpinBox* m_velY {};
    QDoubleSpinBox* m_velZ {};
    QDoubleSpinBox* m_sizeX {};
    QDoubleSpinBox* m_sizeY {};
    QDoubleSpinBox* m_sizeZ {};
    QDoubleSpinBox* m_normalX {};
    QDoubleSpinBox* m_normalY {};
    QDoubleSpinBox* m_normalZ {};
    QDoubleSpinBox* m_massBox {};
    QCheckBox*      m_fixedBox {};
    QDoubleSpinBox* m_stiffnessBox {};
    QDoubleSpinBox* m_dampingBox {};
    QDoubleSpinBox* m_frictionBox {};
    QDoubleSpinBox* m_restitutionBox {};

    // Integrate widget
    QDoubleSpinBox* m_stepDt {};

    // Plots widget
    QLineEdit* m_csvDir {};

    // Console
    QTextEdit* m_console {};

    // Sim control buttons (disabled during run)
    QPushButton* m_btnInit {};
    QPushButton* m_btnStart {};
    QPushButton* m_btnStop {};
    QPushButton* m_btnRun {};
    QPushButton* m_btnPrint {};
    QPushButton* m_btnStep {};
    QPushButton* m_btnApplyConfig {};
    QPushButton* m_btnAdd {};
    QPushButton* m_btnSet {};
    QPushButton* m_btnDel {};

    int m_selectedRow { -1 };
};

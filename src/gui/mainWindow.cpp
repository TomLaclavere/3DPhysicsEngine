/**
 * @file mainWindow.cpp
 * @brief Implementation of the Qt main window: UI construction, slot handlers, and plot integration.
 */
#include "gui/mainWindow.hpp"

#include "gui/flowLayout.hpp"
#include "mathematics/vector.hpp"
#include "objects/object.hpp"
#include "precision.hpp"
#include "utilities/command.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QProcess>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QSplitter>
#include <QStatusBar>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <deque>
#include <sstream>
#include <string>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_config(Config::get())
    , m_world(m_config)
    , m_thread(m_world, this)
{
    buildUi();
}

// ============================================================================
// UI construction
// ============================================================================

/// @brief Build the entire Qt widget tree and connect all signals to slots.
void MainWindow::buildUi()
{
    setWindowTitle("3D Physics Engine");
    // Scrollable central area: last resort when the window is narrower than a single widget
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    setCentralWidget(scroll);
    auto* central = new QWidget;
    scroll->setWidget(central);
    auto* mainLayout = new QHBoxLayout(central);

    // ---- Left panel ----
    auto* leftPanel = new QWidget;
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(6);

    // Simulation controls
    auto* ctrlBox    = new QGroupBox("Simulation");
    auto* ctrlLayout = new FlowLayout(ctrlBox);
    m_btnInit        = new QPushButton("Init");
    m_btnStart       = new QPushButton("Start");
    m_btnStop        = new QPushButton("Stop");
    m_btnRun         = new QPushButton("Run");
    m_btnPrint       = new QPushButton("Print");
    for (auto* btn : { m_btnInit, m_btnStart, m_btnStop, m_btnRun, m_btnPrint })
        ctrlLayout->addWidget(btn);

    // Integrate
    auto* intBox    = new QGroupBox("Integrate");
    auto* intLayout = new FlowLayout(intBox);
    m_stepDt        = new QDoubleSpinBox;
    m_stepDt->setRange(1e-6, 10.0);
    m_stepDt->setDecimals(4);
    m_stepDt->setValue(static_cast<double>(m_config.getTimeStep()));
    m_btnStep = new QPushButton("Step");
    intLayout->addWidget(new QLabel("dt:"));
    intLayout->addWidget(m_stepDt);
    intLayout->addWidget(m_btnStep);

    // Config
    auto* configBox  = new QGroupBox("Config");
    auto* configForm = new QFormLayout(configBox);
    configForm->setRowWrapPolicy(QFormLayout::WrapLongRows);
    m_gravityBox     = new QDoubleSpinBox;
    m_gravityBox->setRange(0.0, 100.0);
    m_gravityBox->setDecimals(4);
    m_gravityBox->setValue(static_cast<double>(m_config.getGravity()));
    m_dtBox = new QDoubleSpinBox;
    m_dtBox->setRange(1e-6, 10.0);
    m_dtBox->setDecimals(6);
    m_dtBox->setValue(static_cast<double>(m_config.getTimeStep()));
    m_durationBox = new QDoubleSpinBox;
    m_durationBox->setRange(0.001, 86400.0);
    m_durationBox->setDecimals(2);
    m_durationBox->setValue(static_cast<double>(m_config.getSimulationDuration()));
    m_solverBox = new QComboBox;
    m_solverBox->addItems({ "Euler", "Verlet", "RK4" });
    m_solverBox->setCurrentText(QString::fromStdString(m_config.getSolver()));
    m_simplifiedCollisionBox = new QCheckBox;
    m_simplifiedCollisionBox->setChecked(m_config.getSimplifiedCollision());
    m_cfgStiffnessBox   = new QDoubleSpinBox;
    m_cfgDampingBox     = new QDoubleSpinBox;
    m_cfgFrictionBox    = new QDoubleSpinBox;
    m_cfgRestitutionBox = new QDoubleSpinBox;
    for (auto* s : { m_cfgStiffnessBox, m_cfgDampingBox })
    {
        s->setRange(0.0, 1e8);
        s->setDecimals(2);
    }
    for (auto* s : { m_cfgFrictionBox, m_cfgRestitutionBox })
    {
        s->setRange(0.0, 1.0);
        s->setDecimals(4);
    }
    m_cfgStiffnessBox->setValue(static_cast<double>(m_config.getDefaultStiffness()));
    m_cfgDampingBox->setValue(static_cast<double>(m_config.getDefaultDamping()));
    m_cfgFrictionBox->setValue(static_cast<double>(m_config.getDefaultFriction()));
    m_cfgRestitutionBox->setValue(static_cast<double>(m_config.getDefaultRestitution()));
    m_btnApplyConfig = new QPushButton("Apply");
    configForm->addRow("Gravity (m/s²):", m_gravityBox);
    configForm->addRow("dt (s):", m_dtBox);
    configForm->addRow("Duration (s):", m_durationBox);
    configForm->addRow("Solver:", m_solverBox);
    configForm->addRow("Simplified collision:", m_simplifiedCollisionBox);
    configForm->addRow(new QLabel("- Default material -"));
    configForm->addRow("Stiffness (N/m):", m_cfgStiffnessBox);
    configForm->addRow("Damping:", m_cfgDampingBox);
    configForm->addRow("Friction:", m_cfgFrictionBox);
    configForm->addRow("Restitution:", m_cfgRestitutionBox);
    configForm->addRow(m_btnApplyConfig);

    // Add object
    auto* addBox    = new QGroupBox("Add Object");
    auto* addLayout = new FlowLayout(addBox);
    m_typeBox       = new QComboBox;
    m_typeBox->addItems({ "sphere", "aabb", "plane" });
    m_nameEdit = new QLineEdit;
    m_nameEdit->setPlaceholderText("name (optional)");
    m_btnAdd = new QPushButton("Add");
    addLayout->addWidget(m_typeBox);
    addLayout->addWidget(m_nameEdit);
    addLayout->addWidget(m_btnAdd);

    // Plots
    auto* plotBox    = new QGroupBox("Plots");
    auto* plotLayout = new QVBoxLayout(plotBox);
    auto* csvRow     = new QHBoxLayout;
    m_csvDir         = new QLineEdit("output/CSV");
    csvRow->addWidget(new QLabel("CSV dir:"));
    csvRow->addWidget(m_csvDir);
    auto* btnRow  = new FlowLayout;
    auto* btnTraj = new QPushButton("Trajectories");
    auto* btnObjs = new QPushButton("3D Objects");
    auto* btnAnim = new QPushButton("Animation");
    btnRow->addWidget(btnTraj);
    btnRow->addWidget(btnObjs);
    btnRow->addWidget(btnAnim);
    plotLayout->addLayout(csvRow);
    plotLayout->addLayout(btnRow);

    leftLayout->addWidget(ctrlBox);
    leftLayout->addWidget(intBox);
    leftLayout->addWidget(configBox);
    leftLayout->addWidget(addBox);
    leftLayout->addWidget(plotBox);
    leftLayout->addStretch();

    // ---- Right panel ----
    auto* rightPanel  = new QWidget;
    auto* rightLayout = new QVBoxLayout(rightPanel);

    // Object table
    m_objectTable = new QTableWidget(0, 6);
    m_objectTable->setHorizontalHeaderLabels({ "ID", "Type", "Pos X", "Pos Y", "Pos Z", "Fixed" });
    m_objectTable->horizontalHeader()->setStretchLastSection(true);
    m_objectTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_objectTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_objectTable->setAlternatingRowColors(true);

    // Selected object properties
    auto* propBox    = new QGroupBox("Selected Object");
    auto* propLayout = new QFormLayout(propBox);
    propLayout->setRowWrapPolicy(QFormLayout::WrapLongRows);
    m_selectedLabel  = new QLabel("No selection");

    auto makeSpin = [](double min, double max, int decimals = 4) -> QDoubleSpinBox*
    {
        auto* s = new QDoubleSpinBox;
        s->setRange(min, max);
        s->setDecimals(decimals);
        return s;
    };
    m_posX           = makeSpin(-1e6, 1e6);
    m_posY           = makeSpin(-1e6, 1e6);
    m_posZ           = makeSpin(-1e6, 1e6);
    m_velX           = makeSpin(-1e6, 1e6);
    m_velY           = makeSpin(-1e6, 1e6);
    m_velZ           = makeSpin(-1e6, 1e6);
    m_normalX        = makeSpin(-1e6, 1e6);
    m_normalY        = makeSpin(-1e6, 1e6);
    m_normalZ        = makeSpin(-1e6, 1e6);
    m_sizeX          = makeSpin(0.0, 1e6);
    m_sizeY          = makeSpin(0.0, 1e6);
    m_sizeZ          = makeSpin(0.0, 1e6);
    m_massBox        = makeSpin(0.0, 1e9);
    m_fixedBox       = new QCheckBox;
    m_stiffnessBox   = makeSpin(0.0, 1e8);
    m_dampingBox     = makeSpin(0.0, 1e8);
    m_frictionBox    = makeSpin(0.0, 1.0);
    m_restitutionBox = makeSpin(0.0, 1.0);
    m_btnSet         = new QPushButton("Set");
    m_btnDel         = new QPushButton("Del");

    // Each row's fields wrap onto several lines when the panel is narrow
    auto flowRow = [](std::initializer_list<QWidget*> widgets) -> QWidget*
    {
        auto* row    = new QWidget;
        auto* layout = new FlowLayout(row);
        layout->setContentsMargins(0, 0, 0, 0);
        for (auto* w : widgets)
            layout->addWidget(w);
        return row;
    };
    propLayout->addRow(m_selectedLabel);
    propLayout->addRow("Pos:", flowRow({ m_posX, m_posY, m_posZ }));
    propLayout->addRow("Vel:", flowRow({ m_velX, m_velY, m_velZ }));
    propLayout->addRow("Normal:", flowRow({ m_normalX, m_normalY, m_normalZ }));
    propLayout->addRow("Size:", flowRow({ m_sizeX, m_sizeY, m_sizeZ }));
    propLayout->addRow("Mass:", flowRow({ m_massBox, new QLabel("Fixed:"), m_fixedBox }));
    propLayout->addRow("Stiffness:", flowRow({ m_stiffnessBox, new QLabel("Damping:"), m_dampingBox }));
    propLayout->addRow("Friction:", flowRow({ m_frictionBox, new QLabel("Restitution:"), m_restitutionBox }));
    propLayout->addRow(flowRow({ m_btnSet, m_btnDel }));

    // Console
    m_console = new QTextEdit;
    m_console->setReadOnly(true);
    m_console->setFontFamily("monospace");

    rightLayout->addWidget(m_objectTable, 2);
    rightLayout->addWidget(propBox);
    rightLayout->addWidget(m_console, 1);

    // Left panel scrolls: it is taller than small screens
    auto* leftScroll = new QScrollArea;
    leftScroll->setWidget(leftPanel);
    leftScroll->setWidgetResizable(true);
    leftScroll->setFrameShape(QFrame::NoFrame);

    // Splitter: the user drags the boundary between panels; extra width goes to the right panel
    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(leftScroll);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(1, 1);
    splitter->setChildrenCollapsible(false);
    mainLayout->addWidget(splitter);

    statusBar()->showMessage("Ready");

    // Start at the content's preferred size, within the screen (the scroll area alone would open small)
    const QSize preferred = central->sizeHint() + QSize(0, statusBar()->sizeHint().height());
    resize(preferred.boundedTo(screen()->availableGeometry().size()));

    // Signals
    connect(m_btnInit, &QPushButton::clicked, this, &MainWindow::onInit);
    connect(m_btnStart, &QPushButton::clicked, this, &MainWindow::onStart);
    connect(m_btnStop, &QPushButton::clicked, this, &MainWindow::onStop);
    connect(m_btnRun, &QPushButton::clicked, this, &MainWindow::onRun);
    connect(m_btnPrint, &QPushButton::clicked, this, &MainWindow::onPrint);
    connect(m_btnStep, &QPushButton::clicked, this, &MainWindow::onStep);
    connect(m_btnApplyConfig, &QPushButton::clicked, this, &MainWindow::onApplyConfig);
    connect(m_btnAdd, &QPushButton::clicked, this, &MainWindow::onAddObject);
    connect(m_btnSet, &QPushButton::clicked, this, &MainWindow::onSetObject);
    connect(m_btnDel, &QPushButton::clicked, this, &MainWindow::onDelObject);
    connect(btnTraj, &QPushButton::clicked, this, &MainWindow::onPlotTrajectories);
    connect(btnObjs, &QPushButton::clicked, this, &MainWindow::onPlotObjects);
    connect(btnAnim, &QPushButton::clicked, this, &MainWindow::onPlotAnimation);
    connect(m_objectTable, &QTableWidget::cellClicked, this, &MainWindow::onObjectTableClicked);
    connect(&m_thread, &SimulationThread::simulationFinished, this, &MainWindow::onSimulationFinished);
}

// ============================================================================
// Helpers
// ============================================================================

/// @brief Rebuild the object table from the current world state.
void MainWindow::refreshObjectTable()
{
    m_objectTable->setRowCount(0);
    for (size_t i = 0; i < m_world.getObjectCount(); ++i)
    {
        const Object* obj = m_world.getObject(i);
        if (!obj)
            continue;

        const int row = m_objectTable->rowCount();
        m_objectTable->insertRow(row);

        const Vector3D pos = obj->getPosition();
        m_objectTable->setItem(row, 0, new QTableWidgetItem(QString::number(obj->getId())));
        m_objectTable->setItem(row, 1,
                               new QTableWidgetItem(QString::fromStdString(toString(obj->getType()))));
        m_objectTable->setItem(
            row, 2, new QTableWidgetItem(QString::number(static_cast<double>(pos.getX()), 'f', 3)));
        m_objectTable->setItem(
            row, 3, new QTableWidgetItem(QString::number(static_cast<double>(pos.getY()), 'f', 3)));
        m_objectTable->setItem(
            row, 4, new QTableWidgetItem(QString::number(static_cast<double>(pos.getZ()), 'f', 3)));
        m_objectTable->setItem(row, 5, new QTableWidgetItem(obj->isFixed() ? "Yes" : "No"));
    }
}

void MainWindow::setControlsEnabled(bool enabled)
{
    for (auto* btn : { m_btnInit, m_btnStart, m_btnStop, m_btnRun, m_btnPrint, m_btnStep, m_btnApplyConfig,
                       m_btnAdd, m_btnSet, m_btnDel })
        btn->setEnabled(enabled);
}

void MainWindow::log(const QString& msg) { m_console->append(msg); }

// ============================================================================
// Slots - simulation control
// ============================================================================

/// @brief Re-initialise the world, clear the table, and reset the selection.
void MainWindow::onInit()
{
    m_world.initialise();
    refreshObjectTable();
    m_selectedRow = -1;
    m_selectedLabel->setText("No selection");
    log("World initialised.");
    statusBar()->showMessage("World initialised");
}

void MainWindow::onStart()
{
    m_world.start();
    log("Simulation started.");
    statusBar()->showMessage("Running");
}

void MainWindow::onStop()
{
    m_world.stop();
    log("Simulation stopped.");
    statusBar()->showMessage("Stopped");
}

/// @brief Start the simulation loop on the background thread. Guards against running when stopped.
void MainWindow::onRun()
{
    if (!m_world.getIsRunning())
    {
        log("Simulation is not running. Click Start first.");
        return;
    }
    setControlsEnabled(false);
    log("Running simulation...");
    statusBar()->showMessage("Simulation running…");
    m_thread.start();
}

/// @brief Re-enable controls and refresh the object table when the simulation thread completes.
void MainWindow::onSimulationFinished()
{
    setControlsEnabled(true);
    refreshObjectTable();
    log("Simulation finished.");
    statusBar()->showMessage("Simulation finished");
}

/// @brief Redirect stdout to capture PhysicsWorld::printState() output into the console widget.
void MainWindow::onPrint()
{
    std::ostringstream oss;
    std::streambuf*    old = std::cout.rdbuf(oss.rdbuf());
    m_world.printState();
    std::cout.rdbuf(old);
    log(QString::fromStdString(oss.str()));
}

/// @brief Read dt from the spinbox, advance one integration step, and refresh the table.
void MainWindow::onStep()
{
    const decimal dt = static_cast<decimal>(m_stepDt->value());
    m_world.setTimeStep(dt);
    m_world.integrate();
    refreshObjectTable();
    log(QString("Integrated one step of %1 s.").arg(m_stepDt->value()));
    statusBar()->showMessage("Stepped");
}

// ============================================================================
// Slots - config
// ============================================================================

/// @brief Read all config spinboxes and apply their values to Config and PhysicsWorld.
void MainWindow::onApplyConfig()
{
    m_world.setGravityCst(static_cast<decimal>(m_gravityBox->value()));
    m_world.setTimeStep(static_cast<decimal>(m_dtBox->value()));
    m_config.setSimulationDuration(static_cast<decimal>(m_durationBox->value()));
    m_world.setSolver(m_solverBox->currentText().toStdString());
    m_config.setSimplifiedCollision(m_simplifiedCollisionBox->isChecked());
    m_config.setDefaultStiffness(static_cast<decimal>(m_cfgStiffnessBox->value()));
    m_config.setDefaultDamping(static_cast<decimal>(m_cfgDampingBox->value()));
    m_config.setDefaultFriction(static_cast<decimal>(m_cfgFrictionBox->value()));
    m_config.setDefaultRestitution(static_cast<decimal>(m_cfgRestitutionBox->value()));
    log(QString("Config applied: g=%1, dt=%2, duration=%3s, solver=%4")
            .arg(m_gravityBox->value())
            .arg(m_dtBox->value())
            .arg(m_durationBox->value())
            .arg(m_solverBox->currentText()));
    statusBar()->showMessage("Config applied");
}

// ============================================================================
// Slots - object management
// ============================================================================

/// @brief Build an add command from the type/name widgets and delegate to handleAddCommand.
void MainWindow::onAddObject()
{
    std::deque<std::string> words;
    words.push_back(m_typeBox->currentText().toStdString());
    const QString name = m_nameEdit->text().trimmed();
    if (!name.isEmpty())
        words.push_back(name.toStdString());

    if (handleAddCommand(m_world, words))
    {
        refreshObjectTable();
        log(QString("Added %1.").arg(m_typeBox->currentText()));
        statusBar()->showMessage("Object added");
    }
    else
    {
        log("Failed to add object.");
    }
}

/// @brief Populate the property form with the selected object's current values.
void MainWindow::onObjectTableClicked(int row, int col)
{
    Q_UNUSED(col)
    m_selectedRow = row;

    const auto* item = m_objectTable->item(row, 0);
    if (!item)
        return;

    const auto    id  = item->text().toUInt();
    const Object* obj = m_world.findById(id);
    if (!obj)
        return;

    m_selectedLabel->setText(
        QString("Object %1 (%2)").arg(id).arg(QString::fromStdString(toString(obj->getType()))));

    const Vector3D pos = obj->getPosition();
    m_posX->setValue(static_cast<double>(pos.getX()));
    m_posY->setValue(static_cast<double>(pos.getY()));
    m_posZ->setValue(static_cast<double>(pos.getZ()));

    const Vector3D vel = obj->getVelocity();
    m_velX->setValue(static_cast<double>(vel.getX()));
    m_velY->setValue(static_cast<double>(vel.getY()));
    m_velZ->setValue(static_cast<double>(vel.getZ()));

    const Vector3D normal = obj->getNormal();
    m_normalX->setValue(static_cast<double>(normal.getX()));
    m_normalY->setValue(static_cast<double>(normal.getY()));
    m_normalZ->setValue(static_cast<double>(normal.getZ()));

    const Vector3D sz = obj->getSize();
    m_sizeX->setValue(static_cast<double>(sz.getX()));
    m_sizeY->setValue(static_cast<double>(sz.getY()));
    m_sizeZ->setValue(static_cast<double>(sz.getZ()));

    m_massBox->setValue(static_cast<double>(obj->getMass()));
    m_fixedBox->setChecked(obj->isFixed());
    m_stiffnessBox->setValue(static_cast<double>(obj->getStiffnessCst()));
    m_dampingBox->setValue(static_cast<double>(obj->getDampingCst()));
    m_frictionBox->setValue(static_cast<double>(obj->getFrictionCst()));
    m_restitutionBox->setValue(static_cast<double>(obj->getRestitutionCst()));
}

/// @brief Read all property widgets and push their values onto the currently selected object.
void MainWindow::onSetObject()
{
    if (m_selectedRow < 0)
        return;

    const auto* item = m_objectTable->item(m_selectedRow, 0);
    if (!item)
        return;

    const auto id  = item->text().toUInt();
    Object*    obj = m_world.findById(id);
    if (!obj)
        return;

    obj->setPosition(Vector3D(static_cast<decimal>(m_posX->value()), static_cast<decimal>(m_posY->value()),
                              static_cast<decimal>(m_posZ->value())));
    obj->setVelocity(Vector3D(static_cast<decimal>(m_velX->value()), static_cast<decimal>(m_velY->value()),
                              static_cast<decimal>(m_velZ->value())));
    obj->setNormal(Vector3D(static_cast<decimal>(m_normalX->value()),
                            static_cast<decimal>(m_normalY->value()),
                            static_cast<decimal>(m_normalZ->value())));
    obj->setSize(Vector3D(static_cast<decimal>(m_sizeX->value()), static_cast<decimal>(m_sizeY->value()),
                          static_cast<decimal>(m_sizeZ->value())));
    obj->setMass(static_cast<decimal>(m_massBox->value()));
    obj->setIsFixed(m_fixedBox->isChecked());
    obj->setStiffnessCst(static_cast<decimal>(m_stiffnessBox->value()));
    obj->setDampingCst(static_cast<decimal>(m_dampingBox->value()));
    obj->setFrictionCst(static_cast<decimal>(m_frictionBox->value()));
    obj->setRestitutionCst(static_cast<decimal>(m_restitutionBox->value()));

    refreshObjectTable();
    log(QString("Object %1 updated.").arg(id));
    statusBar()->showMessage("Object updated");
}

/// @brief Remove the selected object from the world and clear the selection.
void MainWindow::onDelObject()
{
    if (m_selectedRow < 0)
        return;

    const auto* item = m_objectTable->item(m_selectedRow, 0);
    if (!item)
        return;

    const auto id  = item->text().toUInt();
    Object*    obj = m_world.findById(id);
    if (!obj)
        return;

    m_world.removeObject(obj);
    m_selectedRow = -1;
    m_selectedLabel->setText("No selection");
    refreshObjectTable();
    log(QString("Object %1 removed.").arg(id));
    statusBar()->showMessage("Object removed");
}

// ============================================================================
// Slots - plots
// ============================================================================

/**
 * @brief Launch python/generate_plots.py as a detached process.
 * @param mode Plot type passed as CLI argument ("trajectories", "objects", or "animation").
 */
void MainWindow::runPlotScript(const QString& mode)
{
    const QString scriptPath = "python/generate_plots.py";
    const bool    ok         = QProcess::startDetached("python3", { scriptPath, m_csvDir->text(), mode });
    if (ok)
        log(QString("Opening %1 plot in browser...").arg(mode));
    else
        log("Failed to start python3. Make sure it is on PATH and run from the project root.");
}

void MainWindow::onPlotTrajectories() { runPlotScript("trajectories"); }
void MainWindow::onPlotObjects() { runPlotScript("objects"); }
void MainWindow::onPlotAnimation() { runPlotScript("animation"); }

/**
 * @file main_qt.cpp
 * @brief Qt GUI entry point: initialises Config and launches the MainWindow.
 */
#include "gui/mainWindow.hpp"
#include "world/config.hpp"

#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("3D Physics Engine");

    Config& config = Config::get();
    config.loadFromFile("src/config.yaml");
    config.overrideFromCommandLine(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}

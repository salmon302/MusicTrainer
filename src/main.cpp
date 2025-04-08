#include <QApplication>
#include <QApplication>
// #include <QWidget> // No longer needed for placeholder
#include <iostream>

// Include the actual main window header
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // Optional: Set application details
    // QApplication::setApplicationName("MusicTrainer");
    // QApplication::setOrganizationName("YourOrganization"); // Optional

    std::cout << "MusicTrainer Application Starting..." << std::endl;

    // Create and show the main window instance
    MusicTrainer::UI::MainWindow mainWindow;
    mainWindow.show();

    // Placeholder removed
    // QWidget placeholderWindow;
    // placeholderWindow.setWindowTitle("MusicTrainer (Placeholder)");
    // placeholderWindow.setMinimumSize(200, 100);
    // placeholderWindow.show();


    return app.exec();
}
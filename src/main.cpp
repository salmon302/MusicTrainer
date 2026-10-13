#include <QApplication>
#include <iostream>

#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("MusicTrainer");
    QApplication::setOrganizationName("MusicTrainer");

    std::cout << "MusicTrainer Application Starting..." << std::endl;

    MusicTrainer::UI::MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
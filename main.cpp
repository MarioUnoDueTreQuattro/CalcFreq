#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("andreag");
    QCoreApplication::setApplicationName("CalcFreq");
    QApplication::setDesktopFileName("CalcFreq");
    MainWindow w;
    w.show();
    return QApplication::exec();
}

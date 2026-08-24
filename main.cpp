#include "mainwindow.h"

#include <QApplication>
// #include <QDebug>
// #include <QFile>
// #include <QIcon>
// #include <QPixmap>
#include <QLocale>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //qDebug() << "La risorsa SVG esiste?" << QFile::exists(":/icons/calcfreq.svg");
    // Forza l'uso del punto decimale (.) per tutti i widget Qt
    QLocale::setDefault(QLocale::c());
    QCoreApplication::setOrganizationName("andreag");
    QCoreApplication::setApplicationName("CalcFreq");
    QApplication::setDesktopFileName("CalcFreq");
    // Carichiamo l'SVG dentro una QIcon generica
    // QIcon svgIcon(":/icons/calcfreq.svg");
    // // Creiamo un contenitore multi-risoluzione per X11
    // QIcon multiSizeIcon;
    // // Generiamo e aggiungiamo le varie risoluzioni raster
    // int sizes[] = {16, 32, 48, 64, 128, 256};
    // for (int s : sizes) {
    // multiSizeIcon.addPixmap(svgIcon.pixmap(s, s));
    // }
    // // Applichiamo la risorsa multi-dimensione all'applicazione
    // QApplication::setWindowIcon(multiSizeIcon);
    MainWindow w;
    w.show();
    return QApplication::exec();
}

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
//    MainWindow w("127.0.0.1", 5000);
//    w.show();

    return a.exec();
}

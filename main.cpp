#include <QApplication>
#include <QPushButton>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Mainwindow mainwindow;
    mainwindow.show();


    return QApplication::exec();
}
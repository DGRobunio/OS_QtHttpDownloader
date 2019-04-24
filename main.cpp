#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    //将Mainwindow的exitApp信号与QApplication中的quit信号槽进行链接，达到点击指定按钮退出的目的
    QObject::connect(&w, &MainWindow::exitApp, &a, &QApplication::quit);
    w.show();
    return a.exec();
}

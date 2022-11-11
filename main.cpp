#include "mainwindow.h"
#include <QApplication>
#include "dbservicethread.h"
#include "rpcservicethread.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    RPCServiceThread::get_instance()->start();
    DBServiceThread::get_instance()->start();

    MainWindow w;
    w.show();

    return a.exec();
}

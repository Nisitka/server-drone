#include <QApplication>

#include <QDebug>

#include "./server-app/server.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Server server;
    server.runTest();

    return a.exec();
}

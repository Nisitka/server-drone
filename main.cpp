#include <QApplication>

#include <QDebug>

#include "./server-app/server.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    QCoreApplication::addLibraryPath("C:/Qt/6.9.0/mingw_64/plugins");


    Server server(1337);
    server.runTest();

    return a.exec();
}

#include <QCoreApplication>

#ifndef _WIN32
#include <csignal>
#endif

#include <QDebug>

#include "./server-app/server.h"

int main(int argc, char *argv[])
{
//  Игнорируем SIGPIPE только на Linux
#ifndef _WIN32
    std::signal(SIGPIPE, SIG_IGN);
#endif

    QCoreApplication a(argc, argv);

    Server server;
    server.runTest();

    return a.exec();
}

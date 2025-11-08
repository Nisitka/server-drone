#include <QApplication>

#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qDebug() << "Hello, server!";

    return a.exec();
}

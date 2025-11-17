#ifndef CLIENTSMANAGER_H
#define CLIENTSMANAGER_H

#include <QObject>
#include <QMap>

#include "./serversocketadapter.h"

class ClientsManager: public QObject
{
    Q_OBJECT

public slots:

    //
    void initClient(const QString& uuid,
                    ISocketAdapter* clientSock);

public:
    ClientsManager();

private slots:
    void removeClient();

    // Принять сообщение от клиента
    void acceptMessageFromSocket();

private:

    // Клиенты по uuid-м их логинов
    QMap <QString, ISocketAdapter*> clients;

    //
    void processingMessage(const QByteArray& msg);

    // Инициировать отключение клиента от сервера
    void disconnectClient(const QString& uuidClient);
};

#endif // CLIENTSMANAGER_H

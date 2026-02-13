#ifndef CLIENTSMANAGER_H
#define CLIENTSMANAGER_H

#include <QObject>
#include <QMap>

#include "./serversocketadapter.h"
#include "./ActionsClientsManager.h"

class ClientsManager: public QObject
{
    Q_OBJECT
signals:
    // Клиент успешно авторизован и инициализирован
    void itializedClient(ISocketAdapter* clientSock); // по какому сокету

public slots:

public:
    ClientsManager();

    ActionsClientsManager* Actions() const;

private slots:
    //
    void initClient(const QString& uuid,
                    ISocketAdapter* clientSock);

    void removeClient();

    // Принять сообщение от клиента
    void acceptMessageFromSocket();

private:
    ActionsClientsManager* actions;

    // Клиенты по uuid-м их логинов
    QMap <QString, ISocketAdapter*> clients;

    //
    void processingMessage(const QByteArray& msg);

    // Инициировать отключение клиента от сервера
    void disconnectClient(const QString& uuidClient);
};

#endif // CLIENTSMANAGER_H

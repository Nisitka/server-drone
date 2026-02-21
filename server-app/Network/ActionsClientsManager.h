#ifndef ACTIONSCLIENTSMANAGER_H
#define ACTIONSCLIENTSMANAGER_H

#include <QObject>
#include "./serversocketadapter.h"

class ActionsClientsManager: public QObject
{
    Q_OBJECT
signals:
    // Добавить клиента
    void addClient(const QString& login,
                   ISocketAdapter* clientSock);

    // Отправить сообщение клиенту
    void sendByteArray(const QString& login,
                       const QByteArray& data);
};

#endif // ACTIONSCLIENTSMANAGER_H

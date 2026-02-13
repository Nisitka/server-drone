#ifndef ACTIONSCLIENTSMANAGER_H
#define ACTIONSCLIENTSMANAGER_H

#include <QObject>
#include "./serversocketadapter.h"

class ActionsClientsManager: public QObject
{
    Q_OBJECT
signals:
    // Попытаться инициализировать клиента
    void trInitClient(const QString& uuid,
                      ISocketAdapter* clientSock);
};

#endif // ACTIONSCLIENTSMANAGER_H

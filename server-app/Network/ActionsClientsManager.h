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

    // Отправить конкретным пользователям
    void sendByteArrayToUsers(const QStringList& logins,
                              const QByteArray& data);

    // Отправить всем клиентам за исключением определенных
    void sendByteArrayAllUsersExcept(const QStringList& excepted_logins,
                                     const QByteArray& data);
};

#endif // ACTIONSCLIENTSMANAGER_H

#ifndef CLIENTSMANAGER_H
#define CLIENTSMANAGER_H

#include <QObject>
#include <QMap>

#include "./ActionsClientsManager.h"
#include "../DataBase/Tasks/queuetaskdb.h"

class ClientsManager: public QObject
{
    Q_OBJECT
signals:
    // Клиент успешно авторизован и инициализирован
    void itializedClient(ISocketAdapter* clientSock); // по какому сокету

public slots:

public:
    ClientsManager(QueueTaskDB* taskQueue);
    ~ClientsManager();

    ActionsClientsManager* Actions() const;

private slots:
    //
    void initClient(const QString& uuid,
                    ISocketAdapter* clientSock);

    // Отправить данные конкретному пользователю
    void sendByteArray(const QString& login,
                       const QByteArray& data);

    // Отправить всем клиентам за исключением определенных
    void sendByteArrayAllUsersExcept(const QStringList& excepted_logins,
                                     const QByteArray& data);


    // Удалить соединение с клиентом
    void removeClientSocket();

    // Принять сообщение от клиента
    void acceptMessageFromSocket();

private:
    ActionsClientsManager* actions;

    // Добавляем необходимые задачи
    QueueTaskDB* taskQueue;

    // Клиенты по uuid-м их логинов
    QMap <QString, ISocketAdapter*> clients;
    QString socketToLogin(ISocketAdapter*) const;

    //
    void processingMessage(const QByteArray& msg,
                           const QString& login_client);

    // Инициировать отключение клиента от сервера
    void disconnectClient(const QString& uuidClient);
};

#endif // CLIENTSMANAGER_H

#include "clientsmanager.h"

#include <QDebug>

ClientsManager::ClientsManager():
    QObject()
{

}

void ClientsManager::initClient(const QString& uuidClient, ISocketAdapter* socket)
{
    if (clients.contains(uuidClient)){
        qDebug() << "ClientsManager: попытка добавить уже авторизованного клиента -" << uuidClient;
        return ;
    }

    //
    clients[uuidClient] = socket;

    // Отвечаем клиенту на соединение
    ///pSockHandle->sendString("connect");

    // Обработка полученных от клиента сообщений
    connect(socket, &ISocketAdapter::message,
            this,   &ClientsManager::acceptMessageFromSocket);

    connect(socket, &ISocketAdapter::disconnected,
            this,   &ClientsManager::removeClient);
}

void ClientsManager::removeClient()
{
    ISocketAdapter* client = static_cast<ServerSocketAdapter*>(sender());

    /// Ищем нужного клиента и удялем его
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        if (it.value() == client) {
            clients.erase(it);
            break;
        }
    }

    delete client;
    qDebug() << "client removed";
}

void ClientsManager::acceptMessageFromSocket()
{
    ISocketAdapter* client = static_cast<ServerSocketAdapter*>(sender());

    // Сообщение в сыром виде
    const QByteArray& msg = client->getCurrentMessage();

    // Обрабатываем
    if (!msg.isEmpty())
        processingMessage(msg);
    else
        qDebug() << "ClientsManager: попытка обработать пустое сообщение!";
}

void ClientsManager::processingMessage(const QByteArray& msg)
{
    // При const QByteArray& - QIODevice::ReadOnly
    QDataStream stream(msg);

    // Тип принятого сообщения
    uint8_t id_msg;
    stream >> id_msg;
    qDebug() << "processing message, id_message:" << id_msg;
}

void ClientsManager::disconnectClient(const QString& uuidClient)
{
    clients[uuidClient]->disconnect();
}

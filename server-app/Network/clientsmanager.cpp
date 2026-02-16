#include "clientsmanager.h"

#include <QDebug>

//
#include "../DataBase/Tasks/TaskUserLogOut.h"

ClientsManager::ClientsManager(QueueTaskDB* taskQueue_):
    QObject(),
    taskQueue(taskQueue_)
{
    actions = new ActionsClientsManager;
    connect(actions, &ActionsClientsManager::trInitClient,
            this,    &ClientsManager::initClient);
}

ActionsClientsManager* ClientsManager::Actions() const
{
    return actions;
}

void ClientsManager::initClient(const QString& uuidClient, ISocketAdapter* socket)
{
    if (clients.contains(uuidClient)){
        qDebug() << "ClientsManager: попытка добавить уже авторизованного клиента -" << uuidClient;
        return ;
    }

    // Сохраняем клиента
    clients[uuidClient] = socket;

    // Отвечаем клиенту на соединение
    ///pSockHandle->sendString("connect");

    // Обработка полученных от клиента сообщений
    connect(socket, &ISocketAdapter::message,
            this,   &ClientsManager::acceptMessageFromSocket);

    // Удаление клиента при отключении
    connect(socket, &ISocketAdapter::disconnected,
            this,   &ClientsManager::removeClientSocket);

    // Сообщаем, что пользователь инициализирован
    emit itializedClient(socket);
}

void ClientsManager::removeClientSocket()
{
    ISocketAdapter* clientSocket = static_cast<ServerSocketAdapter*>(sender());

    /// Ищем нужного клиента и удялем его
    QString login;
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        if (it.value() == clientSocket) {

            login = it.key();
            clients.erase(it);
            break;
        }
    }

    /// Задача на обновление данных в базе
    if (!login.isEmpty())
        taskQueue->enqueue(new TaskUserLogOut(login));

    delete clientSocket;
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

ClientsManager::~ClientsManager(){
    // Закрываем сессии всех клиентов
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        /// Задача на обновление данных в базе
        taskQueue->enqueue(new TaskUserLogOut(it.key()));
    }
}

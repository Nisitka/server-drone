#include "clientsmanager.h"

#include <QDebug>

//
#include "../DataBase/Tasks/TaskUserLogOut.h"
#include "../../common/protocol/commands_server/command_server.h"

#include "../../common/protocol/commands_client/commands_client_user/commands_client_user_result_auth.h"

using namespace server_protocol;

ClientsManager::ClientsManager(QueueTaskDB* taskQueue_):
    QObject(),
    taskQueue(taskQueue_)
{
    /// Интерфейс по асинхронной работе с клиентами
    actions = new ActionsClientsManager;
    connect(actions, &ActionsClientsManager::addClient,
            this,    &ClientsManager::initClient);
    connect(actions, &ActionsClientsManager::sendByteArray,
            this,    &ClientsManager::sendByteArray);
}

ActionsClientsManager* ClientsManager::Actions() const{
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

    // Обработка полученных от клиента сообщений
    connect(socket, &ISocketAdapter::message,
            this,   &ClientsManager::acceptMessageFromSocket);

    // Удаление клиента при отключении
    connect(socket, &ISocketAdapter::disconnected,
            this,   &ClientsManager::removeClientSocket);

    ///
    command_client_user_result_auth cmd(command_client_user_result_auth::successfully);
    emit socket->trSendByteArray(cmd.toByteArray());

    // Сообщаем, что пользователь инициализирован
    emit itializedClient(socket);
}

void ClientsManager::sendByteArray(const QString& login,
                                   const QByteArray& data)
{
    if (clients.contains(login)){
        ISocketAdapter* clientSocket = clients.value(login);

        if (clientSocket)
            emit clientSocket->trSendByteArray(data);
    }
    else
        qDebug() << "ClientsManager:: try send byteArray unknown client -" << login;
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
    // Тип принятого сообщения
    uint8_t id_msg = protocol_message::get_msg_id(msg);
    qDebug() << "processing message, id_message:" << id_msg;
    if (id_msg != id_msg_command_server){
        qDebug() << "id_msg != id_msg_command_server";
        return;
    }

    /// В зависимости от сообщения инициируем задачу
    // Из данных получаем конкретный номер команды
    uint8_t id_command = get_id_command_server(msg);
    switch (id_command) {
    case id_command_server_map_requreq_objects:
        qDebug() << "id_command_server_map_requreq_objects";

        break;
    default:
        qDebug() << "id_command_server unknown:" << id_command;
        break;
    }

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

#include "clientsmanager.h"

#include <QDebug>

#include "clientsmanager.h"
#include <QDebug>

// Подключаем фабрику серверных команд
#include "../../common/protocol/commands_server/ServerCommandFactory.h"

#include "../DataBase/Tasks/TaskRequreqMapMarkers.h"
#include "../DataBase/Tasks/TaskUpdateMapMarker.h"
#include "../DataBase/Tasks/TaskCreateMapMarker.h"
#include "../DataBase/Tasks/TaskRemoveMapMarker.h"
#include "../DataBase/Tasks/TaskRequredTypesMapMarkers.h"

#include "../../common/protocol/commands_server/command_server.h"
#include "../../common/protocol/commands_server/commands_server_map/command_server_map_object_update.h"
#include "../../common/protocol/commands_server/commands_server_map/command_server_map_object_create.h"
#include "../../common/protocol/commands_server/commands_server_map/command_server_map_remove_object.h"
#include "../../common/protocol/commands_server/commands_server_map/command_server_map_requreq_type_markers.h"

#include "../../common/protocol/commands_client/commands_client_user/commands_client_user_result_auth.h"

using namespace server_protocol;

ClientsManager::ClientsManager(QueueTaskDB* taskQueue_) :
    QObject(),
    taskQueue(taskQueue_)
{
    /// Интерфейс по асинхронной работе с клиентами
    actions = new ActionsClientsManager();
    connect(actions, &ActionsClientsManager::addClient,
            this,    &ClientsManager::initClient);
    connect(actions, &ActionsClientsManager::sendByteArray,
            this,    &ClientsManager::sendByteArray);
    connect(actions, &ActionsClientsManager::sendByteArrayAllUsersExcept,
            this,    &ClientsManager::sendByteArrayAllUsersExcept);
}

ActionsClientsManager* ClientsManager::Actions() const {
    return actions;
}

void ClientsManager::initClient(const QString& uuidClient, const QString& nickname, ISocketAdapter* socket)
{
    if (clients.contains(uuidClient)) {
        qDebug() << "ClientsManager: an attempt to add an already authorized client -" << uuidClient << nickname;
        return;
    }

    // Сохраняем клиента
    clients[uuidClient] = socket;

    // Обработка полученных от клиента сообщений
    connect(socket, &ISocketAdapter::message,
            this,   &ClientsManager::acceptMessageFromSocket);

    // Удаление клиента при отключении
    connect(socket, &ISocketAdapter::disconnected,
            this,   &ClientsManager::removeClientSocket);

    // Отправляем успешный результат авторизации, используя автоматическую упаковку базового класса
    qDebug() << "send command_client_user_result_auth:" << command_client_user_result_auth::successfully << nickname;
    command_client_user_result_auth cmd(command_client_user_result_auth::successfully, nickname);
    emit socket->trSendByteArray(cmd.toByteArray());

    // Сообщаем, что пользователь инициализирован
    emit itializedClient(socket);
}

void ClientsManager::sendByteArray(const QString& login, const QByteArray& data)
{
    if (clients.contains(login)) {
        ISocketAdapter* clientSocket = clients.value(login);
        if (clientSocket)
            emit clientSocket->trSendByteArray(data);
    }
    else {
        qDebug() << "ClientsManager:: try send byteArray unknown client -" << login;
    }
}

void ClientsManager::sendByteArrayAllUsersExcept(const QStringList& excepted_logins, const QByteArray& data)
{
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        const QString login = it.key();
        if (!excepted_logins.contains(login))
            sendByteArray(login, data);
    }
}

void ClientsManager::removeClientSocket()
{
    // Безопасное приведение к вашему типу реализации адаптера (например, SocketAdapter)
    ISocketAdapter* clientSocket = static_cast<ISocketAdapter*>(sender());

    /// Ищем нужного клиента и удаляем его
    QString uuid;
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        if (it.value() == clientSocket) {
            uuid = it.key();
            clients.erase(it);
            break;
        }
    }

    if (!uuid.isEmpty()){
        /// Задача на уведомление об этом других пользователей
        ///
        ///

        // Важно: так как SocketAdapter наследует QObject и имеет внутри deleteLater(),
        // объект безопаснее удалять через deleteLater, а не прямой delete.
        clientSocket->deleteLater();
        qDebug() << "client removed:" << uuid;
    }
}

void ClientsManager::acceptMessageFromSocket()
{
    ISocketAdapter* client = static_cast<ISocketAdapter*>(sender());
    const QString login = socketToLogin(client);

    if (login.isEmpty()) {
        qDebug() << "ClientsManager: A packet has been received from an unauthorized or unknown socket!";
        return;
    }

    // Проверяем тип сообщения верхнего уровня через геттер адаптера
    server_protocol::id_message msgType = client->getLastMsgId();
    if (msgType == server_protocol::id_msg_unknown) {
        qDebug() << "ClientsManager: accept id_msg_unknown!";
        return;
    }

    // Забираем чистое тело сообщения (уже без 4 байт заголовка протокола)
    const QByteArray& bodyData = client->getCurrentMessage();
    if (!bodyData.isEmpty())

        /// В зависимости от типа сообщения
        switch (msgType) {
        case server_protocol::id_msg_heartbeat:
            // Никак не обрабатываем
            qDebug() << "ClientsManager: acceptMessageFromSocket() - accept id_msg_heartbea";
            break;
        case server_protocol::id_msg_command_server:
            processingMsg_command(bodyData, login);
            break;
        default:
            break;
        }

    else{
        qDebug() << "ClientsManager: An empty command body has been received!";
        return;
    }
}

QString ClientsManager::socketToLogin(ISocketAdapter* socket) const
{
    for (auto it = clients.constBegin(); it != clients.constEnd(); ++it) {
        if (it.value() == socket) {
            return it.key();
        }
    }
    return QString();
}

void ClientsManager::processingMsg_command(const QByteArray& bodyData, const QString& uuid_client)
{
    // Задействуем нашу полиморфную фабрику команд для автоматического разбора
    std::unique_ptr<command> incomingCmd = ServerCommandFactory::createCommand(bodyData);

    /// Если такая команда есть команда
    if (incomingCmd) {

        uint8_t id_command = incomingCmd->id_command();
        qDebug() << "ClientsManager: processing the id command =" << id_command << "for client" << uuid_client;

        switch (id_command) {
        case id_command_server_map_requreq_objects: {
            taskQueue->enqueue(new TaskRequreqMapMarkers(Actions(), uuid_client));
            break;
        }
        case id_command_server_map_object_update: {
            // Безопасно приводим указатель к конкретному типу команды, чтобы передать его в задачу БД
            auto* cmd = static_cast<command_server_map_object_update*>(incomingCmd.get());
            // Передаем копию объекта команды (или ссылку, если Task принимает её так)
            taskQueue->enqueue(new TaskUpdateMapMarker(Actions(), uuid_client, *cmd));
            break;
        }
        case id_command_server_map_object_create: {
            auto* cmd = static_cast<command_server_map_object_create*>(incomingCmd.get());
            taskQueue->enqueue(new TaskCreateMapMarker(Actions(), uuid_client, *cmd));
            break;
        }
        case id_command_server_map_object_remove: {
            auto* cmd = static_cast<command_server_map_remove_object*>(incomingCmd.get());
            taskQueue->enqueue(new TaskRemoveMapMarker(Actions(), uuid_client, *cmd));
            break;
        }
        case id_command_server_map_result_requreq_type_markers: {
            auto* cmd = static_cast<command_server_map_requreq_type_markers*>(incomingCmd.get());
            taskQueue->enqueue(new TaskRequredTypesMapMarkers(Actions(), uuid_client, *cmd));
            break;
        }
        default:
            qDebug() << "ClientsManager: For command id =" << id_command << "there is no task handler- data base";
            break;
        }
        // Память объекта incomingCmd автоматически очистится здесь при выходе из области видимости

    }else{
        qWarning() << "ClientsManager: The factory could not recognize or parse the command!";
        return;
    }


}

void ClientsManager::disconnectClient(const QString& uuidClient)
{
    if (clients.contains(uuidClient)) {
        clients[uuidClient]->disconnect();
    }
}

ClientsManager::~ClientsManager()
{
    clients.clear();
}

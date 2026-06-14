#ifndef TASKUSERAUTH_H
#define TASKUSERAUTH_H

#include "./taskdatabase.h"
#include "../../Network/ActionsClientsManager.h"

#include "../../../common/protocol/commands_client/commands_client_user/commands_client_user_result_auth.h"

using namespace server_protocol;

class TaskUserAuth: public TaskDataBase {
public:
    TaskUserAuth(ActionsClientsManager* clientsManager_, ISocketAdapter* socket_,
                 const QString& login_, const QString& pass_) :
        // Экранируем кавычки для защиты от SQL-инъекций
        TaskDataBase("SELECT status_code, user_id FROM __UserAuth('" + QString(login_).replace("'", "''") + "','"
                     + QString(pass_).replace("'", "''") + "')"),
        clientsManager(clientsManager_),
        login(login_),
        socket(socket_)
    { }

    bool processRequestResult(QSqlQuery& query) override final
    {
        if (!query.next()) {
            sendAuthResult(command_client_user_result_auth::invalid);
            return false;
        }

        int code = query.value(0).toInt();

        switch (code) {
        case 0:{
            /// В случае успеха узнаем uuid пользователя
            const QString uuid_user = query.value(1).toString();
            const QString nickname_user = query.value(2).toString();
            qDebug() << uuid_user << nickname_user << login << "- successfully logged into the database! Passing the socket to the ClientsManager.";

            // Инициируем добавление. ClientsManager сам отправит статус successfully в initClient
            emit clientsManager->addClient(uuid_user, nickname_user, socket);
            break;}

        case 1:
            qDebug() << login << "- error login or password!";
            sendAuthResult(command_client_user_result_auth::invalid_login_or_password);
            break;

        case 2:
            qDebug() << "user" << login << "already logged in!";
            sendAuthResult(command_client_user_result_auth::invalid);
            break;

        case 3:
            qDebug() << "error queue from auth" << login;
            sendAuthResult(command_client_user_result_auth::invalid);
            break;

        default:
            qDebug() << "TaskUserAuth: unknown database response code:" << code;
            sendAuthResult(command_client_user_result_auth::invalid);
            return false;
        }

        return true;
    }

private:
    /// Используется только для отправки ошибочных статусов
    void sendAuthResult(command_client_user_result_auth::results_auth status) {
        command_client_user_result_auth cmd(status, "unknown");
        emit socket->trSendByteArray(cmd.toByteArray());
    }

    ActionsClientsManager* clientsManager;
    const QString login;
    ISocketAdapter* socket;
};

#endif // TASKUSERAUTH_H

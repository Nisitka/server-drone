#ifndef TASKUSERAUTH_H
#define TASKUSERAUTH_H

#include "./taskdatabase.h"
#include "../../Network/ActionsClientsManager.h"

#include "../../../common/protocol/commands_client/commands_client_user/commands_client_user_result_auth.h"

using namespace server_protocol;

class TaskUserAuth: public TaskDataBase
{
public:
    TaskUserAuth(ActionsClientsManager* clientsManager_, ISocketAdapter* socket_,
                 const QString& login_, const QString& pass):
        TaskDataBase("SELECT __UserAuth('" + login_ + "','" + pass + "')"),
        clientsManager(clientsManager_),
        login(login_),
        socket(socket_)
    { /* ... */}

    bool processRequestResult(QSqlQuery& query) override final
    {
        if (!query.next()) return false;
        else
        {
            int code = query.value(0).toInt();
            command_client_user_result_auth::results_auth result;
            QByteArray data;

            switch (code) {
            case 0:
                qDebug() << login << "- успешно авторизирован!";
                emit clientsManager->addClient(login, socket);
                break;
            case 1:{
                qDebug() << login << "- error login or password!";
                result = command_client_user_result_auth::invalid_login_or_password;
                command_client_user_result_auth cmd(result);
                emit socket->trSendByteArray(cmd.toByteArray());
                break;}
            case 2:{
                qDebug() << "user" << login << "already logged in!";
                result = command_client_user_result_auth::invalid;
                command_client_user_result_auth cmd(result);
                emit socket->trSendByteArray(cmd.toByteArray());
                break;}
            case 3:{
                qDebug() << "error queue from auth" << login;
                result = command_client_user_result_auth::invalid;
                command_client_user_result_auth cmd(result);
                emit socket->trSendByteArray(cmd.toByteArray());
                break;}

            default:
                qDebug() << "TaskUserAuth: unknown return code query...";
                return false;
            }
        }
        return true;
    }

private:
    ActionsClientsManager* clientsManager;
    const QString login;
    ISocketAdapter* socket;

};

#endif // TASKUSERAUTH_H

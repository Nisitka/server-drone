#ifndef TASKUSERAUTH_H
#define TASKUSERAUTH_H

#include "./taskdatabase.h"
#include "../../Network/ActionsClientsManager.h"

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

            switch (code) {
            case 0:
                qDebug() << login << "- успешно авторизирован!";
                emit clientsManager->trInitClient(login, socket);
                break;
            case 1:
                qDebug() << login << "- error login or password!";
                break;
            case 2:
                qDebug() << "user" << login << "already logged in!";
                break;
            case 3:
                qDebug() << "error queue from auth" << login;
                break;

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

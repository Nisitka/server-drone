#ifndef TASKUSERAUTH_H
#define TASKUSERAUTH_H

#include "./taskdatabase.h"
#include "../../Network/clientsmanager.h"

/// TEST
#include <QThread>

class TaskUserAuth: public TaskDataBase
{
public:
    TaskUserAuth(ClientsManager* clientsManager_, ISocketAdapter* socket_,
                 const QString& login_, const QString& pass):
        TaskDataBase("SELECT __UserAuth('" + login_ + "','" + pass + "')"),
        clientsManager(clientsManager_),
        login(login_),
        socket(socket_)
    { /* ... */}

    bool processRequestResult(QSqlQuery& query) override final
    {
        /// Имитиация долгой обработки
        QThread::msleep(500);

        if (!query.next()) return false;
        else
        {
            int code = query.value(0).toInt();

            switch (code) {
            case 0:
                qDebug() << login << "- успешно авторизирован!";
                clientsManager->initClient(login, socket);
                break;
            case 1:
                qDebug() << login << "- неверный логин или пароль!";
                break;
            case 2:
                qDebug() << "пользователь" << login << "уже в системе!";
                break;

            default:
                qDebug() << "TaskUserAuth: встречен неизвестный код возврата...";
                return false;
            }
        }
        return true;
    }

private:
    ClientsManager* clientsManager;
    const QString login;
    ISocketAdapter* socket;

};

#endif // TASKUSERAUTH_H

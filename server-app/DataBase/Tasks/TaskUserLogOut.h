#ifndef TASKUSERLOGOUT_H
#define TASKUSERLOGOUT_H

#include "../../Network/ActionsClientsManager.h"
#include "./taskdatabase.h"

using namespace server_protocol;

class TaskUserLogOut : public TaskDataBase {
public:
    // Добавляем ActionsClientsManager в конструктор для возможности рассылки сетевых пакетов
    TaskUserLogOut(ActionsClientsManager* clientsManager_, const QString& login_) :
        // Защищаем строку от SQL-инъекций, удваивая одиночные кавычки
        TaskDataBase("SELECT __UserLogOut('" + QString(login_).replace("'", "''") + "')"),
        clientsManager(clientsManager_),
        login(login_)
    { }

    bool processRequestResult(QSqlQuery& query) override final
    {
        if (!query.next()) {
            return false;
        }

        int code = query.value(0).toInt();

        switch (code) {
        case 0:
            qDebug() << login << "- logged out of the session successfully";

            /// (рассылка остальным пользователям):
            // if (clientsManager) {
            //     // Допустим, при выходе пользователя мы удаляем его персональный маркер с карты у всех остальных
            //     command_client_map_object_removed cmd_remove(login); // или UUID маркера этого пользователя
            //
            //     // Отправляем пакет всем, кроме самого отключившегося (его сокет уже закрыт)
            //     emit clientsManager->sendByteArrayAllUsersExcept(QStringList() << login, cmd_remove.toByteArray());
            // }
            break;

        case 1:
            qDebug() << login << "- duplicate session logout";
            break;

        default:
            qDebug() << "TaskUserLogOut: an unknown return code was received....";
            return false;
        }

        return true;
    }

private:
    ActionsClientsManager* clientsManager;
    const QString login;
};
#endif // TASKUSERLOGOUT_H

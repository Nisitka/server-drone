#ifndef TASKUSERLOGOUT_H
#define TASKUSERLOGOUT_H

#include "./taskdatabase.h"

class TaskUserLogOut: public TaskDataBase
{
public:
    TaskUserLogOut(const QString& login_):
        TaskDataBase("SELECT __UserLogOut('" + login_ + "')"),
        login(login_)
    { /* ... */}

    bool processRequestResult(QSqlQuery& query) override final
    {
        if (!query.next()) return false;
        else
        {
            int code = query.value(0).toInt();

            switch (code) {
            case 0:
                qDebug() << login << "- logged out of the session successfully";

                /// Сообщаем об этом другим клиентам или типо того

                break;
            case 1:
                qDebug() << login << "- duplicate session logout";
                break;

            default:
                qDebug() << "TaskUserLogOut: an unknown return code was received....";
                return false;
            }
        }
        return true;
    }

private:
    const QString login;
};

#endif // TASKUSERLOGOUT_H

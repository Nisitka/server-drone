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
                qDebug() << login << "- успешно вышел из сессии";

                /// Сообщаем об этом другим клиентам или типо того


                break;
            case 1:
                qDebug() << login << "- дублирование выхода из сессии";
                break;

            default:
                qDebug() << "TaskUserLogOut: встречен неизвестный код возврата...";
                return false;
            }
        }
        return true;
    }

private:
    const QString login;
};

#endif // TASKUSERLOGOUT_H

#ifndef TASKDATABASE_H
#define TASKDATABASE_H

#include <QString>
#include <QSqlQuery>

// Интерфейс для выполнения задач связанных с БД
class TaskDataBase
{
public:

    // SQL-запрос который посылаем БД
    const QString stringSQL;

    TaskDataBase(const QString& stringSQL_):
        stringSQL(stringSQL_){/* ... */}

    // Для корректного освобождения ресурсов
    virtual ~TaskDataBase(){}

    // Действия, которые производим с результатом SQL-запроса
    virtual bool processRequestResult(QSqlQuery& querry) = 0;
};

#endif // TASKDATABASE_H

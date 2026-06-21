#ifndef TASKDATABASE_H
#define TASKDATABASE_H

#include <QString>
#include <QSqlQuery>

// Интерфейс для выполнения задач связанных с БД
class TaskDataBase
{
public:

    TaskDataBase(const QString& stringSQL_):
        stringSQL(stringSQL_){/* ... */}

    // Для корректного освобождения ресурсов
    virtual ~TaskDataBase(){}

    // Действия, которые производим с результатом SQL-запроса
    virtual bool processRequestResult(QSqlQuery& query) = 0;

    QString getSQL() const{
        return stringSQL;
    }

protected:
    // SQL-запрос который посылаем БД
    QString stringSQL;
};

#endif // TASKDATABASE_H

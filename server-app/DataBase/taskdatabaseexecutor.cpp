#include "taskdatabaseexecutor.h"

#include <QSqlError>
#include <QUuid>

TaskDataBaseExecutor::TaskDataBaseExecutor(QueueTaskDB* taskQueue_):
    taskQueue(taskQueue_),
    connName(QUuid::createUuid().toString(QUuid::WithoutBraces))
{ /* ... */}

bool TaskDataBaseExecutor::connectToDataBase(const QString& host, int port,
                                             const QString& dbName,
                                             const QString& user, const QString& password)
{
    // qDebug() << host << port << dbName << user << password;
    // qDebug() << QSqlDatabase::drivers();


    // Создаем свое соединение с уникальным именем
    db = QSqlDatabase::addDatabase("QPSQL", connName);
    db.setHostName(host);
    db.setPort(port);
    db.setDatabaseName(dbName);
    db.setUserName(user);
    db.setPassword(password);

    if (!db.open()) {
        qDebug() << "BaseExecutor: error connect (" << connName << "):" << db.lastError().text();
        return false;
    }

    return true;
}

void TaskDataBaseExecutor::run(const QString& host, int port,
                               const QString& dbName,
                               const QString& user, const QString& password)
{
    /// Подключение к БД
    if (!connectToDataBase(host, port, dbName, user, password))
    {
        emit error();
        return;
    }

    //
    stopFlag = false;
    while (true) {
        // Проверяем не надо ли нам закончить работу
        {
            QMutexLocker locker(&mutex);
            if (stopFlag) break;
        }

        /// Выполнение задачи
        if (!executeTask(taskQueue->waitDequeue()))
            qDebug() << "TaskDataBaseExecutor: sql-task not execute!";
    }

    // Закрываем соединение с БД
    db.close();
    QSqlDatabase::removeDatabase(db.connectionName());

    emit finished();
}

bool TaskDataBaseExecutor::executeTask(TaskDataBase* task)
{
    const QString SQL_query = task->stringSQL;

    qDebug() << connName << "Start execute sql-task..." << SQL_query;
    bool isExecuted = false;

    /// Если задача не содержит SQL-запрос
    if (SQL_query.isEmpty()){
        qDebug() << "TaskDataBaseExecutor: SQL-query is empty!";
    }
    else{
        // Выполнение SQL-запроса
        QSqlQuery query(db);
        if (query.exec(SQL_query)){

            // Обработка результатов
            if (!task->processRequestResult(query)){
                qDebug() << "TaskDataBaseExecutor: request results processing error:";
            }else
                isExecuted = true;
        }
        else{
            qDebug() << "TaskDataBaseExecutor: error sql-query:" << query.lastError().text();
        }
    }

    /// Освобождаем память от задачи
    delete task;
    return isExecuted;
}

void TaskDataBaseExecutor::stop()
{
    QMutexLocker locker(&mutex);
    stopFlag = true;
}


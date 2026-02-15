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
    qDebug() << host << port << dbName << user << password;
    qDebug() << QSqlDatabase::drivers();


    // Создаем свое соединение с уникальным именем
    db = QSqlDatabase::addDatabase("QPSQL", connName);
    db.setHostName(host);
    db.setPort(port);
    db.setDatabaseName(dbName);
    db.setUserName(user);
    db.setPassword(password);

    if (!db.open()) {
        qDebug() << "Ошибка подключения (" << connName << "):" << db.lastError().text();
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
            qDebug() << "TaskDataBaseExecutor: задача не выполнена!";
    }

    // Закрываем соединение с БД
    db.close();
    QSqlDatabase::removeDatabase(db.connectionName());

    emit finished();
}

bool TaskDataBaseExecutor::executeTask(TaskDataBase* task)
{
    qDebug() << connName << "Начало выполнения задачи..." << task->stringSQL;

    // Выполнение SQL-запроса
    QSqlQuery query(db);
    if (query.exec(task->stringSQL)){

        // Обработка результатов
        if (!task->processRequestResult(query)){
            qDebug() << "TaskDataBaseExecutor: ошибка обработки результатов запроса:";
            return false;
        }
    }
    else{
        qDebug() << "TaskDataBaseExecutor: ошибка запроса:" << query.lastError().text();
        return false;
    }

    /// Освобождаем память от задачи
    delete task;
    return true;
}

void TaskDataBaseExecutor::stop()
{
    QMutexLocker locker(&mutex);
    stopFlag = true;
}


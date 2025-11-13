#include "taskdatabaseexecutor.h"

#include <QSqlError>

TaskDataBaseExecutor::TaskDataBaseExecutor(QueueTaskDB* taskQueue_):
    QObject(),
    taskQueue(taskQueue_)
{ /* ... */}

bool TaskDataBaseExecutor::connectToDataBase(const QString& host, int port,
                                             const QString& dbName,
                                             const QString& user, const QString& password)
{
    // Создаем свое соединение с уникальным именем
    const QString connName = "connetDB-name-" + QString::number(rand()&1000);
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

void TaskDataBaseExecutor::run()
{
    /// Подключение к БД
    if (!connectToDataBase("127.0.0.1", 1234, "nisitka", "postgres", "pass"))
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
    // Выполнение SQL-запроса и обработка результатов
    QSqlQuery query(db);
    if (query.exec(task->stringSQL))
    {
        if (!task->processRequestResult(query))
        {
            qDebug() << "TaskDataBaseExecutor: ошибка обработки результатов запроса:";
            return false;
        }
    }
    else
    {
        qDebug() << "TaskDataBaseExecutor: ошибка запроса:" << query.lastError().text();
        return false;
    }

    return true;
}

void TaskDataBaseExecutor::stop()
{
    QMutexLocker locker(&mutex);
    stopFlag = true;
}


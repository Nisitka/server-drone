#ifndef TASKDATABASEEXECUTOR_H
#define TASKDATABASEEXECUTOR_H

#include <QObject>
#include <QSqlDatabase>

#include "./Tasks/queuetaskdb.h"

#include "../serverErrors.h"

// Обработчик задач
class TaskDataBaseExecutor: public QObject
{
    Q_OBJECT
signals:
    //
    void critical_error(ServerErrors::code_errors code_error, const QString& info = "");

    //
    void finished();
    void started();

public slots:
    void run(const QString& host, int port,
             const QString& dbName,
             const QString& user, const QString& password);
    void stop();

public:
    TaskDataBaseExecutor(QueueTaskDB* taskQueue);

    QString getLastError() const { return lastError; }

private:
    QString lastError;

    QueueTaskDB* taskQueue;

    //
    bool executeTask(TaskDataBase* task);

    //
    QSqlDatabase db;
    bool connectToDataBase(const QString& host, int port,
                           const QString& dbName,
                           const QString& user, const QString& password);
    const QString connName;

    QMutex mutex;
    bool stopFlag;
};

#endif // TASKDATABASEEXECUTOR_H

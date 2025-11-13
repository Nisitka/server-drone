#ifndef QUEUETASKDB_H
#define QUEUETASKDB_H

#include <QMutex>
#include <QQueue>
#include <QWaitCondition>

#include "./taskdatabase.h"

// Потокобезопасная очередь задач для базы данных
class QueueTaskDB
{
public:
    QueueTaskDB() = default;

    // Добавить задачу
    void enqueue(TaskDataBase* task);

    // Ожидать задачу
    TaskDataBase* waitDequeue();

    bool isEmpty() const;
    int count() const;

private:
    mutable QMutex mutex;
    QQueue <TaskDataBase*> tasks;
    QWaitCondition notEmpty; // для ожидания появления задачи
};

#endif // QUEUETASKDB_H

#include "queuetaskdb.h"

void QueueTaskDB::enqueue(TaskDataBase *task)
{
    QMutexLocker locker(&mutex);
    tasks.enqueue(task);
    notEmpty.wakeOne();
}

TaskDataBase* QueueTaskDB::waitDequeue()
{
    QMutexLocker locker(&mutex);

    // Ожидаем появление задачи
    while (tasks.isEmpty()){ /// нужен для защиты от ложного срабатывания
        notEmpty.wait(&mutex);
    }

    return tasks.dequeue();
}

bool QueueTaskDB::isEmpty() const
{
    QMutexLocker locker(&mutex);
    return tasks.isEmpty();
}

int QueueTaskDB::count() const
{
    QMutexLocker locker(&mutex);
    return tasks.size();
}

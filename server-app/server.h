#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QList>

#include "./Network/clientsmanager.h"
#include "./DataBase/taskdatabaseexecutor.h"

class QTcpSocket;
class ISocketAdapter;

class Server: public QTcpServer{
  Q_OBJECT

signals:

  void runSqlExecuters(const QString& host, int port,
                       const QString& dbName,
                       const QString& user, const QString& password);

public:
    explicit Server(int nPort, QObject *parent = nullptr);

    void runTest();

protected:
    //
    void incomingConnection(qintptr socketDescriptor) override final;

private slots:
    void acceptTryAuthMessage();
    void removeNotAuthSocket();

    // Удалить сокет из неавторизованных соединений
    void removeSocketFromNotAuthSockets(ISocketAdapter*);

private:
    // Еще не авторизованные соединения
    QList <ISocketAdapter*> notAuthSockets;

    // Работает с авторизованными пользователями
    ClientsManager* clientsManager;

    // Запуск как сетевой службы
    bool run(int port);

    //
    QueueTaskDB* taskQueue;
    QList <TaskDataBaseExecutor*> sqlExecuters;

    //
    bool readConfig();
    QMap<QString, QString> configParams;
    QStringList expectedKeys = {"host", "port", "database", "user", "password"};
};

#endif // SERVER_H

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
    explicit Server(QObject *parent = nullptr);
    ~Server();

    void runTest();

protected:
    //
    void incomingConnection(qintptr socketDescriptor) override final;

private slots:
    //
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
    bool run();

    //
    QueueTaskDB* taskQueue;
    QList <TaskDataBaseExecutor*> sqlExecuters;

    //
    bool readConfig();
    QMap<QString, QString> configParams;
    QStringList expectedKeys = {"database_host",
                                "database_port",
                                "database",
                                "database_user",
                                "database_password",
                                "port",
                                "executor_count"};
};

#endif // SERVER_H

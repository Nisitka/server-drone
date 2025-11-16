#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QList>

#include "./Network/serversocketadapter.h"

#include "./DataBase/taskdatabaseexecutor.h"

class QTcpServer;
class QTcpSocket;
class ISocketAdapter;

class Server: public QObject {
  Q_OBJECT

signals:

  void runSqlExecuters(const QString& host, int port,
                       const QString& dbName,
                       const QString& user, const QString& password);

public:
  explicit Server(int nPort, QObject *parent = nullptr);

    void runTest();

protected:
  QTcpServer* tcpServer;
  QList<ISocketAdapter*> clients;

private slots:
  void initClient();
  void removeClient();

  // Принять сообщение от клиента
  void acceptMessageFromSocket();

private:
//public: /// для теста!
  //
  void processingMessage(const QByteArray& msg);

  // Инициировать отключение клиента от сервера
  void disconnectClient(ServerSocketAdapter* socket);

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

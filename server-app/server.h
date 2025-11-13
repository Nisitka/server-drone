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
public:
  explicit Server(int nPort, QObject *parent = nullptr);

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
  QueueTaskDB taskQueue;
};

#endif // SERVER_H

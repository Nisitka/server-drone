#include "server.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QString>

#include <QDataStream>

#include <QDebug>

Server::Server(int port, QObject *parent):
    QObject(parent),
    tcpServer(new QTcpServer(this)) {

    // Инициализируем клиента при появлении нового соединения
    connect(tcpServer, &QTcpServer::newConnection,
            this,      &Server::initClient);

    // Обработчики задач связанных с БД
    TaskDataBaseExecutor* executer = new TaskDataBaseExecutor(&taskQueue);
    //executer.

    if (run(port))
    {

    }
}

bool Server::run(int port)
{
    if (tcpServer->listen(QHostAddress::Any, port)) {

    }
    else
    {
        tcpServer->close();

        qDebug() << "Server: не удалось запустить сервер - " << tcpServer->errorString();
        return false;
    }

    return true;
}

void Server::initClient()
{
    qDebug() << "init new connection...";

    QTcpSocket* pclientSock = tcpServer->nextPendingConnection();
    ISocketAdapter *pSockHandle = new ServerSocketAdapter(pclientSock);

    clients.push_back(pSockHandle);

    // Отвечаем клиенту на соединение
    ///pSockHandle->sendString("connect");

    // Обработка полученных от клиента сообщений
    connect(pSockHandle, &ISocketAdapter::message,
          this,        &Server::acceptMessageFromSocket);


    connect(pSockHandle, &ISocketAdapter::disconnected,
            this,        &Server::removeClient);
}

void Server::removeClient()
{
  ISocketAdapter* client = static_cast<ServerSocketAdapter*>(sender());

  clients.removeOne(client);

  delete client;
  qDebug() << "client removed";
}

void Server::acceptMessageFromSocket()
{
    ISocketAdapter* client = static_cast<ServerSocketAdapter*>(sender());

    // Сообщение в сыром виде
    const QByteArray& msg = client->getCurrentMessage();

    // Обрабатываем
    if (!msg.isEmpty())
        processingMessage(msg);
    else
        qDebug() << "Server::" << "попытка обработать пустое сообщение!";
}

void Server::processingMessage(const QByteArray& msg)
{
    uint8_t id_msg;

    // При const QByteArray& - QIODevice::ReadOnly
    QDataStream stream(msg);

    stream >> id_msg;
    qDebug() << "processing message:" << id_msg;
}

void Server::disconnectClient(ServerSocketAdapter* socket)
{
    socket->disconnect();
}

//void Server::message(QString msg) {
//  foreach (ISocketAdapter *sock, m_clients)
//    sock->sendString(msg);
//}

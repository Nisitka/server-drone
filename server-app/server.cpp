#include "server.h"

#include <QApplication>
#include <QThread>

#include <QTcpServer>
#include <QTcpSocket>
#include <QString>

#include <QDataStream>
#include <QFile>

#include <QDebug>

Server::Server(int port, QObject *parent):
    QObject(parent),
    tcpServer(new QTcpServer(this)) {

    qDebug() << "Hello, drone-server!";

    // Инициализируем клиента при появлении нового соединения
    connect(tcpServer, &QTcpServer::newConnection,
            this,      &Server::initClient);

    //
    taskQueue = new QueueTaskDB;

    // Обработчики задач связанных с БД
    for (int i=0; i<4; i++)
    {
        TaskDataBaseExecutor* executer = new TaskDataBaseExecutor(taskQueue);
        connect(this,     &Server::runSqlExecuters,
                executer, &TaskDataBaseExecutor::run);

        QThread* thread = new QThread;
        executer->moveToThread(thread);
        thread->start();

        sqlExecuters.append(executer);
    }

    if (readConfig() && run(port)){
        qDebug() << "Server: успешно запущен!";
    }
}

void Server::runTest()
{
    while (true){
        //taskQueue.enqueue();
    }
}

bool Server::run(int port)
{
    if (tcpServer->listen(QHostAddress::Any, port)) {
        emit runSqlExecuters(configParams["host"],
                             configParams["port"].toInt(),
                             configParams["database"],
                             configParams["user"],
                             configParams["password"]);
    }
    else
    {
        tcpServer->close();

        qDebug() << "Server: не удалось запустить сервер - " << tcpServer->errorString();
        return false;
    }

    return true;
}

bool Server::readConfig()
{
    QFile file(QApplication::applicationDirPath() + "/conf/config.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл config.txt";
        return false;
    }

    // Ищем параметры в формате key=value
    configParams.clear();
    while (!file.atEnd()) {
        QString line = file.readLine().trimmed();

        // Игнорируем пустые строки и комментарии (если есть)
        if (line.isEmpty() || line.startsWith('#'))
            continue;

        QStringList parts = line.split('=', Qt::KeepEmptyParts);
        if (parts.size() == 2) {
            QString key = parts.at(0).trimmed();
            QString value = parts.at(1).trimmed();
            configParams[key] = value;
        }
    }
    file.close();

    // Проверка того, что считали
    for (const QString &key: std::as_const(expectedKeys)) {
        if (configParams.contains(key)) {
            qDebug() << key << ":" << configParams[key];
        } else {
            qWarning() << "Параметр" << key << "не найден в config.txt";
            return false;
        }
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

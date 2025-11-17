#include "server.h"

#include <QApplication>
#include <QThread>

#include <QTcpServer>
#include <QTcpSocket>
#include <QString>

#include <QDataStream>
#include <QFile>

#include <QDebug>

#include "../common/protocol/command_server_user.h"
#include "../server-app/DataBase/Tasks/TaskUserAuth.h"

using namespace server_protocol;

Server::Server(int port, QObject *parent):
    QTcpServer(parent)
{
    qDebug() << "Hello, drone-server!";

    //
    taskQueue = new QueueTaskDB;

    //
    clientsManager = new ClientsManager;
    QThread* thread = new QThread(this);
    clientsManager->moveToThread(thread);
    thread->start();

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

void Server::incomingConnection(qintptr socketDescriptor)
{
    // Создаем сокет в основном потоке, привязываем к нему дескриптор
    QTcpSocket* socket = new QTcpSocket;
    socket->setSocketDescriptor(socketDescriptor);

    // Добавляем в список пытающихся ломиться в систему
    ISocketAdapter* socketAdapter = new ServerSocketAdapter(socket);
    notAuthSockets.append(socketAdapter);

    // Взаимодействие неавторизованного клиента
    connect(socketAdapter, &ISocketAdapter::message,
            this,   &Server::acceptTryAuthMessage);
    connect(socketAdapter, &ISocketAdapter::disconnected,
            this,          &Server::removeNotAuthSocket);
}

void Server::acceptTryAuthMessage()
{
    ISocketAdapter* client = static_cast<ServerSocketAdapter*>(sender());

    // Сообщение в сыром виде
    const QByteArray& msg = client->getCurrentMessage();

    // Обрабатываем
    if (!msg.isEmpty()){
        // При const QByteArray& - QIODevice::ReadOnly
        QDataStream stream(msg);

        // Тип принятого сообщения
        uint8_t id_msg;
        stream >> id_msg;

        // Проверяем что это требуемый тип команды
        if (id_msg == id_msg_command_server_user){
            // Извлекаем оставшиеся данные
            QByteArray data;
            stream >> data; /// копирование!!!

            // Из данных получаем конкретный номер команды
            id_command_server_user id_com = command_server_user::get_command_id(data);

            // Исходя из номеры команды создаем объект-команду
            if (id_com == id_command_server_user_auth){
                command_server_user_auth command(data); /// копирование в конструкторе!!!

                // Добавляем задачу на авторизацию пользователя
                TaskDataBase* task = new TaskUserAuth(
                    clientsManager, client, command.Login(), command.Password()); /// копирование в конструкторе!!!
                taskQueue->enqueue(task);
            }
            else{
                qDebug() << "Server: от неавторизованного пользователя получилии команду" << id_com <<", ожидая команду" << id_command_server_user_auth;
            }
        }
        else{
            qDebug() << "Server: от неавторизованного пользователя получили сообщение не того типа!";
        }
    }
    else
        qDebug() << "Server: попытка обработать пустое сообщение!";
}

void Server::removeNotAuthSocket()
{
    ISocketAdapter* client = static_cast<ServerSocketAdapter*>(sender());
    disconnect(client, &ISocketAdapter::message,
               this,   &Server::acceptTryAuthMessage);
    disconnect(client, &ISocketAdapter::disconnected,
               this,          &Server::removeNotAuthSocket);

    notAuthSockets.removeAll(client);
    delete client;
}

void Server::runTest()
{
    ISocketAdapter* client = new ServerSocketAdapter(new QTcpSocket);

    int i = 0;
    while (i < 1000){
        qDebug() << i;
        command_server_user_auth command("djigurda", "12345678");
        // Добавляем задачу на авторизацию пользователя
        TaskDataBase* task = new TaskUserAuth(
            clientsManager, client, command.Login(), command.Password()); /// копирование в конструкторе!!!
        taskQueue->enqueue(task);
        QThread::msleep(1);

        i++;
    }
}

bool Server::run(int port)
{
    if (listen(QHostAddress::Any, port)) {
        emit runSqlExecuters(configParams["host"],
                             configParams["port"].toInt(),
                             configParams["database"],
                             configParams["user"],
                             configParams["password"]);
    }
    else
    {
        this->close();

        qDebug() << "Server: не удалось запустить сервер - " << this->errorString();
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

void Server::authorClient(QTcpSocket* clientSock)
{
    qDebug() << "init new connection...";

    /// Создание задачи на авторизацию

}

//void Server::message(QString msg) {
//  foreach (ISocketAdapter *sock, m_clients)
//    sock->sendString(msg);
//}

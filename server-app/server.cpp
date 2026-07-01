#include "server.h"

#include <QApplication>
#include <QThread>

#include <QTcpServer>
#include <QTcpSocket>
#include <QString>

#include <QDataStream>
#include <QFile>

#include <QDebug>
#include <iostream>

// Подключаем фабрику серверных команд
#include "../common/protocol/commands_server/ServerCommandFactory.h"

#include "../common/protocol/commands_server/command_server.h"
#include "../common/protocol/commands_server/commands_server_user/command_server_user_auth.h"

#include "../server-app/DataBase/Tasks/TaskUserAuth.h"

using namespace server_protocol;

Server::Server(QObject *parent) :
    QTcpServer(parent)
{
    qDebug() << "Hello, drone-server!";

    // Очередь задач
    taskQueue = new QueueTaskDB;

    // Агрегатор пользователей
    clientsManager = new ClientsManager(taskQueue);

    // Взаимодействие
    // Удаляем соединения из списка неавторизованных, когда ClientsManager их успешно авторизует
    connect(clientsManager, &ClientsManager::itializedClient,
            this,           &Server::removeSocketFromNotAuthSockets);

    QThread* thread = new QThread(this);
    clientsManager->moveToThread(thread);
    thread->start();

    /// Запускаемся по параметрам conf файла
    if (readConfig()) {
        // Обработчики задач связанных с БД
        int executor_count = configParams["executor_count"].toInt();
        if (executor_count > 20) executor_count = 20;

        for (int i = 0; i < executor_count; i++) {
            /// Запуск обработчика задач
            TaskDataBaseExecutor* executer = new TaskDataBaseExecutor(taskQueue);
            connect(this,     &Server::runSqlExecuters,
                    executer, &TaskDataBaseExecutor::run);

            // Контроль критических ошибок
            connect(executer, &TaskDataBaseExecutor::critical_error,
                    this,     &Server::shutdown);

            QThread* thread = new QThread(this); // Добавлен родитель для контроля памяти threads
            executer->moveToThread(thread);
            thread->start();

            sqlExecuters.append(executer);

            /// Задержка
            QThread::msleep(200);
        }

        if (run())
            qDebug() << "Server: successfully start!";
        else
            qDebug() << "Server: error run!";
    }
}

void Server::shutdown(ServerErrors::code_errors code, const QString& info)
{
    // Вывод текста ошибки в стандартный поток ошибок
    switch (code) {
    case ServerErrors::connect_to_database:
        std::cerr << "[CRITICAL] - error connect to database:" << info.toStdString() << std::endl;
        break;
    default:
        std::cerr << "[CRITICAL] - unknown error..." << info.toStdString() << std::endl;
        break;
    }

    // Завершение с кодом ошибки для systemd
    std::exit(EXIT_FAILURE);
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    // Создаем сокет в основном потоке, привязываем к нему дескриптор
    QTcpSocket* socket = new QTcpSocket;
    socket->setSocketDescriptor(socketDescriptor);
    qDebug() << "new connect:" << socket->peerAddress().toString();

    // Добавляем в список пытающихся ломиться в систему
    ISocketAdapter* socketAdapter = new ServerSocketAdapter(socket);
    notAuthSockets.append(socketAdapter);

    /// Взаимодействие неавторизованного клиента
    // Попытаться авторизовать клиента через сокет
    connect(socketAdapter, &ISocketAdapter::message,
            this,   &Server::acceptTryAuthMessage);
    // Удаление неавторизованного сокета при дисконнекте
    connect(socketAdapter, &ISocketAdapter::disconnected,
            this,          &Server::removeNotAuthSocket);
}

void Server::acceptTryAuthMessage()
{
    ISocketAdapter* clientSocket = static_cast<ISocketAdapter*>(sender());

    // Запрашиваем тип сообщения верхнего уровня через геттер интерфейса
    id_message msgType = clientSocket->getLastMsgId();


    // Проверяем, что прилетела именно серверная команда
    if (msgType != id_msg_command_server) {

        if (msgType == server_protocol::id_msg_heartbeat)
            qDebug() << "Server: accept id_msg_heartbeat from an unauthorized user";
        else
            qDebug() << "Server: A command was received from an unauthorized user, but the id_message != id_msg_command_server";
        return;
    }

    // Сообщение в чистом виде (bodyData без 4 байт заголовка протокола)
    const QByteArray bodyData = clientSocket->getCurrentMessage();
    qDebug() << "size AuthMessage body -" << bodyData.size();

    if (bodyData.isEmpty()) {
        qDebug() << "Server: An attempt to process an empty message body!";
        return;
    }

    // Передаем чистое тело в фабрику для автоматического полиморфного разбора
    std::unique_ptr<command> incomingCmd = ServerCommandFactory::createCommand(bodyData);

    if (!incomingCmd) {
        qWarning() << "Server: The factory could not parse the incoming authorization bytes!";
        return;
    }

    uint8_t id_command = incomingCmd->id_command();

    // Исходя из номера команды создаем задачу для БД
    if (id_command == id_command_server_user_auth) {

        // Безопасно приводим к конкретному классу команды авторизации
        auto* authCmd = static_cast<command_server_user_auth*>(incomingCmd.get());

        qDebug() << "try user auth:" << authCmd->Login() << authCmd->Password();

        // Добавляем задачу на авторизацию пользователя в очередь БД
        TaskDataBase* task = new TaskUserAuth(
            clientsManager->Actions(), clientSocket,
            authCmd->Login(), authCmd->Password()
            );
        taskQueue->enqueue(task);
    }
    else {
        qDebug() << "Server: unauthorized user sent command, id =" << id_command
                 << ", but the server strictly expects the authorization command (id =" << id_command_server_user_auth << ")";
    }
    // Память объекта incomingCmd автоматически очистится здесь
}

void Server::removeNotAuthSocket()
{
    ISocketAdapter* client = static_cast<ISocketAdapter*>(sender());
    qDebug() << "disconnect not auth socket";

    removeSocketFromNotAuthSockets(client);

    client->deleteLater();
}

void Server::removeSocketFromNotAuthSockets(ISocketAdapter* client)
{
    disconnect(client, &ISocketAdapter::message,
               this,   &Server::acceptTryAuthMessage);
    disconnect(client, &ISocketAdapter::disconnected,
               this,   &Server::removeNotAuthSocket);

    notAuthSockets.removeAll(client);
}

void Server::runTest()
{

}

bool Server::run()
{
    if (listen(QHostAddress::Any, configParams["port"].toInt())) {
        emit runSqlExecuters(configParams["database_host"],
                             configParams["database_port"].toInt(),
                             configParams["database"],
                             configParams["database_user"],
                             configParams["database_password"]);
    }
    else {
        this->close();
        qDebug() << "Server: couldn't start - " << this->errorString();
        return false;
    }
    return true;
}

bool Server::readConfig()
{
    QFile file(QApplication::applicationDirPath() + "/conf/config.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Couldn't open the file config.txt";
        return false;
    }

    configParams.clear();
    while (!file.atEnd()) {
        QString line = file.readLine().trimmed();

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

    for (const QString &key : std::as_const(expectedKeys)) {
        if (configParams.contains(key)) {
            qDebug() << key << ":" << configParams[key];
        } else {
            qWarning() << "Parameter" << key << "not found in config.txt";
            return false;
        }
    }

    return true;
}

Server::~Server() {
    // Чистим неавторизованные сокеты
    qDeleteAll(notAuthSockets);
    notAuthSockets.clear();

    delete clientsManager;
    delete taskQueue;
}


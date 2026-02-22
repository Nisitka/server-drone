#include "server.h"

#include <QApplication>
#include <QThread>

#include <QTcpServer>
#include <QTcpSocket>
#include <QString>

#include <QDataStream>
#include <QFile>

#include <QDebug>

#include "../common/protocol/commands_server/command_server.h"
#include "../common/protocol/commands_server/commands_server_user/command_server_user_auth.h"

#include "../server-app/DataBase/Tasks/TaskUserAuth.h"

/// TEST
#include "../server-app/DataBase/Tasks/TaskUserLogOut.h"

using namespace server_protocol;

Server::Server(QObject *parent):
    QTcpServer(parent)
{
    qDebug() << "Hello, drone-server!";

    // Очередь задач
    taskQueue = new QueueTaskDB;

    // Агригатор пользователей
    clientsManager = new ClientsManager(taskQueue);
    /// Взаимодействие
    // Удаляем соединения из списка неавторизованных
    connect(clientsManager, &ClientsManager::itializedClient,
            this,           &Server::removeSocketFromNotAuthSockets);
    QThread* thread = new QThread(this);
    clientsManager->moveToThread(thread);
    thread->start();

    /// Запускаемся на по параметрам conf файла
    if (readConfig()){
        // Обработчики задач связанных с БД
        int executor_count = configParams["executor_count"].toInt();
        if (executor_count > 8) executor_count = 8;

        for (int i=0; i<executor_count; i++){
            TaskDataBaseExecutor* executer = new TaskDataBaseExecutor(taskQueue);
            connect(this,     &Server::runSqlExecuters,
                    executer, &TaskDataBaseExecutor::run);

            QThread* thread = new QThread;
            executer->moveToThread(thread);
            thread->start();

            sqlExecuters.append(executer);
        }

        if (run())
            qDebug() << "Server: successfully start!";
        else
            qDebug() << "Server: error run!";
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

    /// Взаимодействие неавторизованного клиента
    // Попытаться авторизовать клиента через сокет
    connect(socketAdapter, &ISocketAdapter::message,
            this,   &Server::acceptTryAuthMessage);
    // Удаление неавторизованного сокета
    connect(socketAdapter, &ISocketAdapter::disconnected,
            this,          &Server::removeNotAuthSocket);
}

void Server::acceptTryAuthMessage()
{
    ISocketAdapter* clientSocket = static_cast<ServerSocketAdapter*>(sender());

    // Сообщение в сыром виде
    const QByteArray msg = clientSocket->getCurrentMessage();
    qDebug() << "size AuthMessage -" << msg.size() << msg;

    // Обрабатываем
    if (!msg.isEmpty()){
        // Тип принятого сообщения
        uint8_t id_msg = protocol_message::get_msg_id(msg);

        // Проверяем что это требуемый тип команды
        if (id_msg == id_msg_command_server){
            qDebug() << "Server: accept msg:" << id_msg_command_server;

            // Из данных получаем конкретный номер команды
            uint8_t id_command = get_id_command_server(msg);

            // Исходя из номера команды создаем объект-команду
            if (id_command == id_command_server_user_auth){
                // Декодируем данные в команду
                command_server_user_auth command(msg); /// копирование в конструкторе!!!

                qDebug() << "try user auth:" << command.Login() << command.Password();
                TaskDataBase* task;

                // Добавляем задачу на авторизацию пользователя в очередь
                task = new TaskUserAuth(
                    clientsManager->Actions(), clientSocket,
                    command.Login(), command.Password()); /// копирование в конструкторе!!!
                taskQueue->enqueue(task);
            }
            else{
                qDebug() << "Server: от неавторизованного пользователя получилии команду" << id_command <<", ожидая команду" << id_command_server_user_auth;
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

    removeSocketFromNotAuthSockets(client);
    delete client;
}

void Server::removeSocketFromNotAuthSockets(ISocketAdapter* client)
{
    disconnect(client, &ISocketAdapter::message,
               this,   &Server::acceptTryAuthMessage);
    disconnect(client, &ISocketAdapter::disconnected,
               this,          &Server::removeNotAuthSocket);

    notAuthSockets.removeAll(client);
}

void Server::runTest()
{
    //ISocketAdapter* client = new ServerSocketAdapter(new QTcpSocket);

    // for (int i=0; i<1000; i++){
    //     command_server_user_auth command("djigurda", "12345678");
    //     TaskDataBase* task = nullptr;

    //     if (i%2)
    //         task = new TaskUserAuth(clientsManager->Actions(), client,
    //                                 command.Login(), command.Password());
    //     else
    //         task = new TaskUserLogOut(command.Login());

    //     if (task)
    //         taskQueue->enqueue(task);

    //     QThread::msleep(1);
    // }

    /// Завершаем все сессии
    command_server_user_auth command("djigurda", "12345678");
    TaskDataBase* task = new TaskUserLogOut(command.Login());
    taskQueue->enqueue(task);
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
    else
    {
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
            qWarning() << "Parameter" << key << "not found in config.txt";
            return false;
        }
    }

    return true;
}

Server::~Server(){
    delete clientsManager;
}

//void Server::message(QString msg) {
//  foreach (ISocketAdapter *sock, m_clients)
//    sock->sendString(msg);
//}

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

Server::Server(int port, QObject *parent):
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

    // Обработчики задач связанных с БД
    for (int i=0; i<4; i++){
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
        uint8_t id_msg = static_cast<uint8_t>(msg[0]);

        // Проверяем что это требуемый тип команды
        if (id_msg == id_msg_command_server){
            qDebug() << "accept msg:" << id_msg_command_server;

            // Извлекаем оставшиеся данные
            QByteArray data = msg.mid(1); /// копирование!!!

            // Из данных получаем конкретный номер команды
            uint8_t id_com = command_server::get_command_id(data);

            // Исходя из номера команды создаем объект-команду
            if (id_com == id_command_server_user_auth){
                // Декодируем данные в команду
                command_server_user_auth command(data); /// копирование в конструкторе!!!

                qDebug() << "try user auth:" << command.Login() << command.Password();
                TaskDataBase* task;

                // Добавляем задачу на авторизацию пользователя в очередь
                task = new TaskUserAuth(
                    clientsManager->Actions(), clientSocket,
                    command.Login(), command.Password()); /// копирование в конструкторе!!!
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
    ISocketAdapter* client = new ServerSocketAdapter(new QTcpSocket);

    command_server_user_auth command("djigurda", "12345678");
    TaskDataBase* task = nullptr;
    task = new TaskUserLogOut(command.Login());
    if (task)
        taskQueue->enqueue(task);
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

Server::~Server(){
    delete clientsManager;
}

//void Server::message(QString msg) {
//  foreach (ISocketAdapter *sock, m_clients)
//    sock->sendString(msg);
//}

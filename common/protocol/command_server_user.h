#ifndef COMMAND_SERVER_USER_H
#define COMMAND_SERVER_USER_H

#include "./protocol_message.h"
#include <QDataStream>
#include <QIODevice>

namespace server_protocol {

// Какие есть команды для сервера
enum id_command_server_user: uint8_t{
    id_command_server_user_auth
};

class command_server_user{
public:

    // Узнать какая команда связанная
    // с картой сервера заложена в сообщение
    static id_command_server_user get_command_id(const QByteArray& data) {
        // id команды лежит в самом начале
        return (id_command_server_user)static_cast<uint8_t>(data[0]);
    }
};

// Авторизоваться на сервере:
// логин (QString), пароль (QString)
class command_server_user_auth{
public:

    // data разбиваются на свойства команды
    command_server_user_auth(const QByteArray& data)
    {
        // Считываем поля (при const QByteArray& - QIODevice::ReadOnly)
        QDataStream stream(data);
        stream.device()->seek(1); // минуем id команды
        stream >> login >> pass;
    }

    command_server_user_auth(const QString& login_, const QString& pass_):
        login(login_), pass(pass_)
    { /* ... */}

    void toByteArray(QByteArray& boxForData) const
    {
        QDataStream stream(&boxForData, QIODevice::WriteOnly);
        stream << (uint8_t)id_command_server_user_auth /// id команды, который не храним
               << login
               << pass;
    }

    const QString& Login() const{
        return login;
    }
    const QString& Password() const{
        return pass;
    }

private:
    QString login;
    QString pass;
};

}
#endif // COMMAND_SERVER_USER_H

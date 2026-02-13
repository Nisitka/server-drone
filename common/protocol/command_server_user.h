#ifndef COMMAND_SERVER_USER_H
#define COMMAND_SERVER_USER_H

#include "./protocol_message.h"
#include "./command_server.h"

namespace server_protocol {


// Авторизоваться на сервере:
// логин (QString), пароль (QString)
class command_server_user_auth: public command_server{
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

    void toByteArray(QByteArray& boxForData) const override final
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

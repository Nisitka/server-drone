#ifndef COMMAND_SERVER_USER_AUTH_H
#define COMMAND_SERVER_USER_AUTH_H

#include "../command_server.h"

namespace server_protocol {

// Авторизоваться на сервере:
// логин (QString), пароль (QString)
class command_server_user_auth: public command_server{
public:

    // data разбиваются на свойства команды
    command_server_user_auth(const QByteArray& data):
        command_server(id_command_server_user_auth)
    {
        // Считываем поля (при const QByteArray& - QIODevice::ReadOnly)
        QDataStream stream(data);
        stream.device()->seek(1); // минуем id команды
        //uint8_t id_cmd;
        //stream >> id_cmd;
        stream >> login >> pass;
    }

    command_server_user_auth(const QString& login_, const QString& pass_):
        command_server(id_command_server_user_auth),
        login(login_), pass(pass_)
    { /* ... */}

    void toByteArray(QByteArray& boxForData) const override final
    {
        QDataStream stream(&boxForData, QIODevice::WriteOnly);
        stream << (uint8_t)id_cmd
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

#endif // COMMAND_SERVER_USER_AUTH_H

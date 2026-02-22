#ifndef COMMAND_SERVER_USER_AUTH_H
#define COMMAND_SERVER_USER_AUTH_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"

namespace server_protocol {

// Авторизоваться на сервере:
// логин (QString), пароль (QString)
class command_server_user_auth: public protocol_message,
                                public command{
public:

    // data разбиваются на свойства команды
    command_server_user_auth(const QByteArray& data):
        protocol_message(id_msg_command_server),
        command(id_command_server_user_auth)
    {
        int readStrPos = sizeof(uint8_t)*2; // минуем id_msg, id_cmd
        int pos_end;
        login = readStringFromByteArray(data, pos_end, readStrPos);
        pass  = readStringFromByteArray(data, pos_end, pos_end);
    }

    command_server_user_auth(const QString& login_, const QString& pass_):
        protocol_message(id_msg_command_server),
        command(id_command_server_user_auth),
        login(login_), pass(pass_)
    { /* ... */}

    QByteArray toByteArray() const override final{
        QByteArray byteArray;

        byteArray.append(static_cast<char>(id_msg));
        byteArray.append(static_cast<char>(id_cmd));

        // Прикладные данные
        appendStringToByteArray(login, byteArray);
        appendStringToByteArray(pass, byteArray);

        return byteArray;
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

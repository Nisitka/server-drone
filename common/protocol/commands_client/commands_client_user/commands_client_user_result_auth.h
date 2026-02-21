#ifndef COMMANDS_CLIENT_USER_RESULT_AUTH_H
#define COMMANDS_CLIENT_USER_RESULT_AUTH_H

#include "../command_client.h"

namespace server_protocol {

class command_client_user_result_auth: public command_client{
public:
    enum results_auth: uint8_t{
        successfully,
        invalid_login_or_password,
        invalid
    };

    command_client_user_result_auth(const QByteArray& data):
        command_client(id_command_client_user_result_auth)
    {
        QDataStream stream(data);
        // минуем id команды
        uint8_t id;
        stream >> id;

        stream >> value;
    }

    command_client_user_result_auth(results_auth value_):
        command_client(id_command_client_user_result_auth),
        value(value_)
    { /* ... */}

    void toByteArray(QByteArray& boxForData) const override final
    {
        QDataStream stream(&boxForData, QIODevice::WriteOnly);
        stream << (uint8_t)id_cmd
               << value;
    }

    uint8_t Value() const{
        return value;
    }

private:
    uint8_t value;
};

}
#endif // COMMANDS_CLIENT_USER_RESULT_AUTH_H

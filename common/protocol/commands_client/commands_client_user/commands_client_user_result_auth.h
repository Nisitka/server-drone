#ifndef COMMANDS_CLIENT_USER_RESULT_AUTH_H
#define COMMANDS_CLIENT_USER_RESULT_AUTH_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"

#include "../command_client.h"

namespace server_protocol {

class command_client_user_result_auth: public protocol_message,
                                       public command{
public:
    enum results_auth: uint8_t{
        successfully,
        invalid_login_or_password,
        invalid
    };

    command_client_user_result_auth(const QByteArray& data):
        protocol_message(id_msg_command_client),
        command(id_command_client_user_result_auth)
    {
        int posData = sizeof(uint8_t)*2; // минуем id_msg, id_cmd

        const char* dataPtr = data.constData();
        memcpy(&value, dataPtr + posData, sizeof(value));
    }

    command_client_user_result_auth(results_auth value_):
        protocol_message(id_msg_command_client),
        command(id_command_client_user_result_auth),
        value(value_)
    { /* ... */}

    QByteArray toByteArray() const override final
    {
        QByteArray byteArray;

        byteArray.append(static_cast<char>(id_msg));
        byteArray.append(static_cast<char>(id_cmd));

        byteArray.append(static_cast<char>(value));

        return byteArray;
    }

    uint8_t Value() const{
        return value;
    }

private:
    uint8_t value;
};

}
#endif // COMMANDS_CLIENT_USER_RESULT_AUTH_H

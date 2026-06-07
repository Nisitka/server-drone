#ifndef COMMAND_SERVER_USER_REQUREQ_INFO_USERS_H
#define COMMAND_SERVER_USER_REQUREQ_INFO_USERS_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"

namespace server_protocol {

// Запросить информацию об пользователях
class command_server_user_requreq_info_users: public protocol_message, public command {

    command_server_user_requreq_info_users(/*const QByteArray& bodyData*/):
        protocol_message(id_msg_command_server),
        command(id_command_server_user_requreq_info_users)
    {
        /*
        // Нам нужно сохранить пришедшие байты в базовый класс
        this->data = bodyData;

        int offset = 0;

        // Сначала считываем id_cmd (1 байт), так как он идет первым в теле команды
        if (offset + sizeof(uint8_t) <= static_cast<size_t>(data.size())) {
            // Если id_cmd нужно проверить или сохранить, считываем его.
            // просто сдвигаем offset
            offset += sizeof(uint8_t);
        }
        */
    }

};

}

#endif // COMMAND_SERVER_USER_REQUREQ_INFO_USERS_H

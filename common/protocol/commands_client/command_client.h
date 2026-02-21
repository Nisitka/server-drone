#ifndef COMMAND_CLIENT_H
#define COMMAND_CLIENT_H

#include "../protocol_message.h"
#include "../command.h"

namespace server_protocol {

// Какие есть команды для клиента
enum id_command_client: uint8_t{
    //
    id_command_client_unknown,

    /// Команды, связанные с пользователями
    id_command_client_user_result_auth,

    /// Команды, связанные с картой
};

class command_client: public command{
public:
    command_client(uint8_t id_cmd = id_command_client_unknown): command(id_cmd){

    }

    static uint8_t get_command_id(const QByteArray& data) {

        if (!data.isEmpty()) {

            // id команды лежит в самом начале
            return static_cast<uint8_t>(data[0]);
        }
        else {
            qDebug() << "get_command_id: data.isEmpty()!";
            return  id_command_client_unknown;
        }
    }
};

}

#endif // COMMAND_CLIENT_H

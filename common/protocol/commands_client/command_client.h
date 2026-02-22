#ifndef COMMAND_CLIENT_H
#define COMMAND_CLIENT_H

#include <QByteArray>
#include <QDebug>

namespace server_protocol {

// Какие есть команды для клиента
enum id_command_client: uint8_t{
    //
    id_command_client_unknown,

    /// Команды, связанные с пользователями
    id_command_client_user_result_auth,

    /// Команды, связанные с картой
};

inline uint8_t get_id_command_client(const QByteArray& data_msg) {
    if (!data_msg.isEmpty()) {
        // id сообщения лежит в самом начале
        return static_cast<uint8_t>(data_msg[1]);
    }
    else {
        qDebug() << "get_msg_id: data_msg.isEmpty()!";
        return  id_command_client_unknown;
    }
}

}

#endif // COMMAND_CLIENT_H

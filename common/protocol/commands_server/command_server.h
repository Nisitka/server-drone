#ifndef COMMAND_SERVER_H
#define COMMAND_SERVER_H

#include <QByteArray>
#include <QDebug>

namespace server_protocol {

// Какие есть команды для сервера
enum id_command_server: uint8_t{
    //
    id_command_server_unknown,

    /// Команды, связанные с пользователями
    id_command_server_user_auth,

    /// Команды, связанные с картой
    id_command_server_map_object_create,
    id_command_server_map_object_remove,
    id_command_server_map_object_set_position,
    id_command_server_map_requreq_objects
};

inline uint8_t get_id_command_server(const QByteArray& data_msg) {
    if (!data_msg.isEmpty()) {
        // id сообщения лежит в самом начале
        return static_cast<uint8_t>(data_msg[1]);
    }
    else {
        qDebug() << "get_msg_id: data_msg.isEmpty()!";
        return  id_command_server_unknown;
    }
}

}

#endif // COMMAND_SERVER_H

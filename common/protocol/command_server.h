#ifndef COMMAND_SERVER_H
#define COMMAND_SERVER_H

#include "./protocol_message.h"
#include <QDataStream>
#include <QIODevice>

namespace server_protocol {

// Какие есть команды для сервера
enum id_command_server: uint8_t{
    /// Команды, связанные с пользователями
    id_command_server_user_auth,

    /// Команды, связанные с картой
    id_command_server_map_object_create,
    id_command_server_map_object_remove,
    id_command_server_map_object_set_position,
    id_command_server_map_requreq_objects
};

class command_server{
public:
    void virtual toByteArray(QByteArray& boxForData) const = 0;

    // Узнать какая команда связанна
    // с картой сервера заложена в сообщение
    static id_command_server get_command_id(const QByteArray& data) {
        // id команды лежит в самом начале
        return (id_command_server)static_cast<uint8_t>(data[0]);
    }

};

}

#endif // COMMAND_SERVER_H

#ifndef COMMAND_SERVER_H
#define COMMAND_SERVER_H

#include "../protocol_message.h"
#include <QDataStream>
#include <QIODevice>

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

class command_server{
public:
    command_server(uint8_t id_cmd_ = id_command_server_unknown): id_cmd(id_cmd_) {}
    void virtual toByteArray(QByteArray& boxForData) const = 0;

    uint8_t id_command() const {return id_cmd;}

    // Узнать какая команда связанна
    // с картой сервера заложена в сообщение
    static uint8_t get_command_id(const QByteArray& data) {

        qDebug() << "??????????????" << data.size();
        if (!data.isEmpty()) {
            qDebug() << "*****************" << static_cast<uint8_t>(data[0]);
            // id команды лежит в самом начале
            return static_cast<uint8_t>(data[0]);
        }
        else {
            qDebug() << "get_command_id: data.isEmpty()!";
            return id_command_server_unknown;
        }
    }

protected:
    const uint8_t id_cmd;
};

}

#endif // COMMAND_SERVER_H

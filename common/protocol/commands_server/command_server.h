#ifndef COMMAND_SERVER_H
#define COMMAND_SERVER_H

#include <QByteArray>

#include <QDebug>

namespace server_protocol {

// Какие есть команды для сервера
enum id_command_server: uint8_t{
    //
    id_command_server_unknown = 0,

    /// Команды, связанные с пользователями
    id_command_server_user_auth               = 10,
    id_command_server_user_requreq_info_users = 11,

    /// ------- Команды, связанные с картой -------------------
    // Метки
    id_command_server_map_object_create        = 101,
    id_command_server_map_object_remove        = 102,
    id_command_server_map_object_update        = 103,
    id_command_server_map_object_set_position  = 104,
    id_command_server_map_requreq_objects      = 100,

    // Типы меток
    id_command_server_map_requreq_type_markers        = 110,
    id_command_server_map_result_requreq_type_markers = 111,
    id_command_server_map_create_type_markers         = 112,
    id_command_server_map_remove_type_markers         = 113,
    id_command_server_map_update_type_markers         = 114,
    /// -------------------------------------------------------
};

/**
 * @brief Безопасное извлечение ID серверной команды из чистого тела пакета
 */
inline id_command_server get_id_command_server(const QByteArray& bodyData) {
    // В теле команды должен быть как минимум 1 байт (сам id_cmd)
    if (bodyData.size() >= 1) {
        return static_cast<id_command_server>(bodyData[0]);
    } else {
        qDebug() << "get_id_command_server: bodyData слишком мал или пуст!";
        return id_command_server_unknown;
    }
}

}

#endif // COMMAND_SERVER_H

#ifndef COMMAND_CLIENT_H
#define COMMAND_CLIENT_H

#include "../command.h"

#include <QByteArray>

#include <QDebug>

namespace server_protocol {

// Какие есть команды для клиента
enum id_command_client : uint8_t {
    id_command_client_unknown,

    /// Команды, связанные с пользователями
    id_command_client_user_result_auth,

    /// Команды, связанные с картой
    id_command_client_map_object_created,
    id_command_client_map_object_update,
    id_command_client_map_object_removed,
    id_command_client_map_result_requreq_markers,
    id_command_client_map_requreq_data_markers
};

/**
 * @brief Безопасное извлечение ID клиентской команды из чистого тела пакета (bodyData)
 */
inline id_command_client get_id_command_client(const QByteArray& bodyData) {
    // В теле команды должен быть как минимум 1 байт (сам id_cmd)
    if (bodyData.size() >= 1) {
        return static_cast<id_command_client>(bodyData[0]);
    } else {
        qDebug() << "get_id_command_client: bodyData слишком мал или пуст!";
        return id_command_client_unknown;
    }
}

}

#endif // COMMAND_CLIENT_H

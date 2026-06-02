#ifndef SERVERCOMMANDFACTORY_H
#define SERVERCOMMANDFACTORY_H

#include "./commands_server_map/command_server_map_object_create.h"
#include "./commands_server_map/command_server_map_object_update.h"
#include "./commands_server_map/command_server_map_remove_object.h"
#include "./commands_server_map/command_server_map_requreq_objects.h"

#include "./commands_server_user/command_server_user.h"
#include "./commands_server_user/command_server_user_auth.h"

#include <memory>

namespace server_protocol {

/**
 * @brief Фабрика для автоматического создания объектов команд
 */
class ServerCommandFactory {
public:
    // Функция принимает чистый bodyData из сокета, определяет id_cmd
    // и возвращает готовый объект нужного класса, обернутый в unique_ptr.
    static std::unique_ptr<command> createCommand(const QByteArray& bodyData) {
        id_command_server cmdId = get_id_command_server(bodyData);

        switch (cmdId) {
        case id_command_server_user_auth:
            return std::make_unique<command_server_user_auth>(bodyData);

        case id_command_server_map_object_create:
            return std::make_unique<command_server_map_object_create>(bodyData);

        case id_command_server_map_object_update:
            return std::make_unique<command_server_map_object_update>(bodyData);

        case id_command_server_map_object_remove:
            return std::make_unique<command_server_map_remove_object>(bodyData);

        case id_command_server_map_requreq_objects:
            return std::make_unique<command_server_map_requreq_objects>(bodyData);

        case id_command_server_map_object_set_position:
            // Здесь будет ваш класс для установки позиции, когда вы его напишете
            // return std::make_unique<command_server_map_object_set_position>(bodyData);
            qWarning() << "Фабрика: Команда set_position еще не реализована";
            return nullptr;

        case id_command_server_unknown:
        default:
            qWarning() << "Фабрика: Получен неизвестный id_cmd =" << static_cast<uint8_t>(cmdId);
            return nullptr;
        }
    }
};

}

#endif // SERVERCOMMANDFACTORY_H

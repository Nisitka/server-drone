#ifndef CLIENTCOMMANDFACTORY_H
#define CLIENTCOMMANDFACTORY_H

#include "../commands_client/commands_client_user/commands_client_user_result_auth.h"

#include "../commands_client/commands_client_map/command_client_map_result_requreq_markers.h"
#include "../commands_client/commands_client_map/command_client_map_requreq_data_markers.h"
#include "../commands_client/commands_client_map/command_client_map_object_created.h"
#include "../commands_client/commands_client_map/command_client_map_requreq_data_markers.h"
#include "../commands_client/commands_client_map/command_client_map_object_removed.h"
#include "../commands_client/commands_client_map/command_client_map_object_update.h"

#include <memory>

namespace server_protocol {

class ClientCommandFactory {
public:
    /**
     * @brief Автоматически создает объект конкретной клиентской команды из байт сети
     */
    static std::unique_ptr<command> createCommand(const QByteArray& bodyData) {
        id_command_client cmdId = get_id_command_client(bodyData);

        switch (cmdId) {
        case id_command_client_user_result_auth:
            return std::make_unique<command_client_user_result_auth>(bodyData);

        case id_command_client_map_object_created:
            return std::make_unique<command_client_map_object_created>(bodyData);

        case id_command_client_map_object_update:
            return std::make_unique<command_client_map_object_update>(bodyData);

        case id_command_client_map_object_removed:
            return std::make_unique<command_client_map_object_removed>(bodyData);

        case id_command_client_map_result_requreq_markers:
            return std::make_unique<command_client_map_result_requreq_markers>(bodyData);

        case id_command_client_map_requreq_data_markers:
            return std::make_unique<command_client_map_requreq_data_markers>(bodyData);

        case id_command_client_unknown:
        default:
            qWarning() << "Фабрика клиента: Получен неизвестный или необрабатываемый id_cmd ="
                       << static_cast<uint8_t>(cmdId);
            return nullptr;
        }
    }
};

}

#endif // CLIENTCOMMANDFACTORY_H

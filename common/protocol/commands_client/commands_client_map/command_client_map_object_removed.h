#ifndef COMMAND_CLIENT_MAP_OBJECT_REMOVED_H
#define COMMAND_CLIENT_MAP_OBJECT_REMOVED_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_client/command_client.h"

namespace server_protocol {

class command_client_map_object_removed:public protocol_message,
                                        public command{
public:
    command_client_map_object_removed(const QByteArray& data):
        protocol_message(id_msg_command_client),
        command(id_command_client_map_object_removed),
        uuid_object(readStringFromByteArray(data, sizeof(uint8_t)*2))// минуем id_msg, id_cmd
    { /* ... */ }

    command_client_map_object_removed(const QString& uuid_object_):
        protocol_message(id_msg_command_client),
        command(id_command_client_map_object_removed),
        uuid_object(uuid_object_)
    { /* ... */ }

    QByteArray toByteArray() const override final{
        QByteArray byteArray;

        byteArray.append(static_cast<char>(id_msg));
        byteArray.append(static_cast<char>(id_cmd));

        appendStringToByteArray(uuid_object, byteArray);

        return byteArray;
    }

    const QString uuid_object;
};

}

#endif // COMMAND_CLIENT_MAP_OBJECT_REMOVED_H

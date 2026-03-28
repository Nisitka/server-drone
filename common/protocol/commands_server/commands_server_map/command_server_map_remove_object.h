#ifndef COMMAND_SERVER_MAP_REMOVE_OBJECT_H
#define COMMAND_SERVER_MAP_REMOVE_OBJECT_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"

namespace server_protocol {

class command_server_map_remove_object: public protocol_message,
                                        public command{
public:

    command_server_map_remove_object(const QByteArray& data):
        protocol_message(id_msg_command_server),
        command(id_command_server_map_object_remove),
        uuid_marker(readStringFromByteArray(data, sizeof(uint8_t)*2)) // минуем id_msg, id_cmd
    { /* ... */ }

    command_server_map_remove_object(const QString& uuid):
    protocol_message(id_msg_command_server),
    command(id_command_server_map_object_remove),
    uuid_marker(uuid)
    { /* ... */ }

    QByteArray toByteArray() const override final{
        QByteArray byteArray;

        byteArray.append(static_cast<char>(id_msg));
        byteArray.append(static_cast<char>(id_cmd));

        appendStringToByteArray(uuid_marker, byteArray);

        return byteArray;
    }

    const QString uuid_marker;
};

}

#endif // COMMAND_SERVER_MAP_REMOVE_OBJECT_H

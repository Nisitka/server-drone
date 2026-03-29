#ifndef COMMAND_CLIENT_MAP_OBJECT_CREATED_H
#define COMMAND_CLIENT_MAP_OBJECT_CREATED_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_client/command_client.h"

#include "../../common/data/data_map_marker.h"

namespace server_protocol {

class command_client_map_object_created:public protocol_message,
                                        public command{
public:
    command_client_map_object_created(const QByteArray& data):
        protocol_message(id_msg_command_client),
        command(id_command_client_map_object_created),
        data_marker(data, sizeof(uint8_t)*2) // минуем id_msg, id_cmd
    { /* ... */}

    command_client_map_object_created(const data_map_marker& data_marker_):
        protocol_message(id_msg_command_client),
        command(id_command_client_map_object_created),
        data_marker(data_marker_)
    { /* ... */}

    QByteArray toByteArray() const override final{
        QByteArray byteArray;

        byteArray.append(static_cast<char>(id_msg));
        byteArray.append(static_cast<char>(id_cmd));

        data_marker.appendToByteArray(byteArray);

        return byteArray;
    }

    const data_map_marker data_marker;
};

}

#endif // COMMAND_CLIENT_MAP_OBJECT_CREATED_H

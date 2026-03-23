#ifndef COMMAND_SERVER_MAP_OBJECT_UPDATE_H
#define COMMAND_SERVER_MAP_OBJECT_UPDATE_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"

#include "../../common/data/data_map_marker.h"

namespace server_protocol {

class command_server_map_object_update: public protocol_message,
                                        public command{
public:
    command_server_map_object_update(const QByteArray& data):
        protocol_message(id_msg_command_server),
        command(id_command_server_map_object_update),
        data_marker(data, sizeof(uint8_t)*2) // минуем id_msg, id_cmd
    { /* ... */}

    command_server_map_object_update(const data_map_marker& data):
        protocol_message(id_msg_command_server),
        command(id_command_server_map_object_update),
        data_marker(data)
    { /* ... */}

    QByteArray toByteArray() const override final{
        QByteArray byteArray;

        byteArray.append(static_cast<char>(id_msg));
        byteArray.append(static_cast<char>(id_cmd));

        data_marker.appendToByteArray(byteArray);

        return byteArray;
    }

private:
    data_map_marker data_marker;
};

}

#endif // COMMAND_SERVER_MAP_OBJECT_UPDATE_H

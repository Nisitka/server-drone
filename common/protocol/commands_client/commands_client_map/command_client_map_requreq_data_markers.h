#ifndef COMMAND_CLIENT_MAP_REQUREQ_DATA_MARKERS_H
#define COMMAND_CLIENT_MAP_REQUREQ_DATA_MARKERS_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"

#include "../command_client.h"

#include "../../common/data/data_map_marker.h"

namespace server_protocol {

class command_client_map_requreq_data_markers:  public protocol_message,
                                                public command{
public:
    command_client_map_requreq_data_markers(const QByteArray& data):
        protocol_message(id_msg_command_client),
        command(id_command_client_map_requreq_data_markers),
        data_marker(data, sizeof(uint8_t)*2) // минуем id_msg, id_cmd
    {

    }

    command_client_map_requreq_data_markers(const data_map_marker& marker):
        protocol_message(id_msg_command_client),
        command(id_command_client_map_requreq_data_markers),
        data_marker(marker) // минуем id_msg, id_cmd
    {

    }

    QByteArray toByteArray() const override final
    {
        QByteArray byteArray;

        byteArray.append(static_cast<char>(id_msg));
        byteArray.append(static_cast<char>(id_cmd));

        data_marker.appendToByteArray(byteArray);

        return byteArray;
    }

    const data_map_marker& dataMarker() const{
        return data_marker;
    }

private:
    const data_map_marker data_marker;
};

}

#endif // COMMAND_CLIENT_MAP_REQUREQ_DATA_MARKERS_H

#ifndef COMMAND_CLIENT_MAP_RESULT_REQUREQ_MARKERS_H
#define COMMAND_CLIENT_MAP_RESULT_REQUREQ_MARKERS_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"

#include "../command_client.h"

namespace server_protocol {

class command_client_map_result_requreq_markers: public protocol_message,
                                                 public command{
public:
    enum results_requreq: uint8_t{
        successfully,
        invalid
    };

    uint8_t getResult() const{
        return result;
    }
    uint8_t getCountMarkers() const{
        return count_markers;
    }

    command_client_map_result_requreq_markers(const QByteArray& data):
        protocol_message(id_msg_command_client),
        command(id_command_client_map_result_requreq_markers)
    {
        int posData = sizeof(uint8_t)*2; // минуем id_msg, id_cmd

        const char* dataPtr = data.constData();

        memcpy(&result, dataPtr + posData, sizeof(result));
        posData += sizeof(result);

        memcpy(&count_markers, dataPtr + posData, sizeof(count_markers));
    }

    command_client_map_result_requreq_markers(results_requreq result_,
                                              uint8_t count_markers_):
        protocol_message(id_msg_command_client),
        command(id_command_client_map_result_requreq_markers),
        result(result_),
        count_markers(count_markers_)
    {

    }

    QByteArray toByteArray() const override final
    {
        QByteArray byteArray;

        byteArray.append(static_cast<char>(id_msg));
        byteArray.append(static_cast<char>(id_cmd));

        byteArray.append(static_cast<char>(result));
        byteArray.append(static_cast<char>(count_markers));

        return byteArray;
    }

private:
    uint8_t result;
    uint8_t count_markers;
};

}

#endif // COMMAND_CLIENT_MAP_RESULT_REQUREQ_MARKERS_H

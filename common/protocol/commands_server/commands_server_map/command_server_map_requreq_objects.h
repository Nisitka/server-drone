#ifndef COMMAND_SERVER_MAP_REQUREQ_OBJECTS_H
#define COMMAND_SERVER_MAP_REQUREQ_OBJECTS_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"

namespace server_protocol {

class command_server_map_requreq_objects:   public protocol_message,
                                            public command{
public:

    command_server_map_requreq_objects(/*const QByteArray& data*/):
        protocol_message(id_msg_command_server),
        command(id_command_server_map_requreq_objects)
    {
        /* ... */
    }

    QByteArray toByteArray() const override final{
        QByteArray byteArray;

        byteArray.append(static_cast<char>(id_msg));
        byteArray.append(static_cast<char>(id_cmd));

        return byteArray;
    }

private:

};

}

#endif // COMMAND_SERVER_MAP_REQUREQ_OBJECTS_H

#ifndef COMMAND_H
#define COMMAND_H

#include "./protocol_message.h"
#include <QDataStream>
#include <QIODevice>

namespace server_protocol {

class command{
public:
    command(uint8_t id_cmd_): id_cmd(id_cmd_){
        /* ... */
    }

    void virtual toByteArray(QByteArray& boxForData) const = 0;

    uint8_t id_command() const {return id_cmd;}

protected:
    const uint8_t id_cmd;
};

}

#endif // COMMAND_H

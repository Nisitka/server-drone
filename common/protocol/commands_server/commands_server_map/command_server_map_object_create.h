#ifndef COMMAND_SERVER_MAP_OBJECT_CREATE_H
#define COMMAND_SERVER_MAP_OBJECT_CREATE_H

#include <QDataStream>
#include <QIODevice>

#include <QColor>

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"

#include "../../common/data/data_map_marker.h"

namespace server_protocol {

// Создать объект на карте:
class command_server_map_object_create: public protocol_message,
                                        public command{
public:

    // data разбиваются на свойства команды
    command_server_map_object_create(const QByteArray& data):
        protocol_message(id_msg_command_server),
        command(id_command_server_map_object_create),
        data_marker(data, sizeof(uint8_t)*2) // минуем id_msg, id_cmd
    {

    }

    command_server_map_object_create(const data_map_marker& data):
        protocol_message(id_msg_command_server),
        command(id_command_server_map_object_create),
        data_marker(data)
    { /* ... */}

    QByteArray toByteArray() const override final{
        QByteArray byteArray;

        byteArray.append(static_cast<char>(id_msg));
        byteArray.append(static_cast<char>(id_cmd));

        data_marker.appendToByteArray(byteArray);

        return byteArray;
    }

    QString UUID() const{
        return data_marker.uuid;
    }

    uint8_t type_object() const{
        return data_marker.type_obj_id;
    }
    uint8_t subtype_object() const{
        return data_marker.subtype_obj_id;
    }

    QString Name() const{
        return data_marker.name;
    }

    QString Info() const{
        return data_marker.info;
    }

    QColor ColorName() const{
        return data_marker.ColorName();
    }
    uint8_t ColorName_R() const{
        return data_marker.color_name_r;
    }
    uint8_t ColorName_G() const{
        return data_marker.color_name_g;
    }
    uint8_t ColorName_B() const{
        return data_marker.color_name_b;
    }

    double Lat() const{
        return data_marker.lat;
    }
    double Lon() const{
        return data_marker.lon;
    }

private:
    data_map_marker data_marker;
};

}

#endif // COMMAND_SERVER_MAP_OBJECT_CREATE_H

#ifndef COMMAND_SERVER_MAP_OBJECT_CREATE_H
#define COMMAND_SERVER_MAP_OBJECT_CREATE_H

#include <QDataStream>
#include <QIODevice>

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"

#include "./command_server_map.h"

namespace server_protocol {

// Создать объект на карте:
// тип ебъекта(uint8_t), координаты(double,double), имя (QString)
class command_server_map_object_create: public protocol_message,
                                        public command{
public:

    // data разбиваются на свойства команды
    command_server_map_object_create(const QByteArray& data):
        protocol_message(id_msg_command_server),
        command(id_command_server_map_object_create)
    {
        int posData = sizeof(uint8_t)*2; // минуем id_msg, id_cmd

        const char* dataPtr = data.constData();

        memcpy(&type_obj, dataPtr + posData, sizeof(type_obj));
        posData += sizeof(type_obj);

        memcpy(&lat, dataPtr + posData, sizeof(lat));
        posData += sizeof(lat);

        memcpy(&lon, dataPtr + posData, sizeof(lon));
    }

    command_server_map_object_create(type_object_map type_obj_, double Lat, double Lon):
        protocol_message(id_msg_command_server),
        command(id_command_server_map_object_create),
        type_obj(type_obj_),
        lat(Lat), lon(Lon)
    { /* ... */}

    QByteArray toByteArray() const override final{
        QByteArray byteArray;

        byteArray.append(static_cast<char>(id_msg));
        byteArray.append(static_cast<char>(id_cmd));

        byteArray.append(static_cast<char>(type_obj));
        byteArray.append(reinterpret_cast<const char*>(&lat), sizeof(lat));
        byteArray.append(reinterpret_cast<const char*>(&lon), sizeof(lon));

        return byteArray;
    }

    uint8_t type_object() const{
        return type_obj;
    }
    double Lat() const{
        return lat;
    }
    double Lon() const{
        return lon;
    }

private:
    uint8_t type_obj;
    double lat;
    double lon;
};

}

#endif // COMMAND_SERVER_MAP_OBJECT_CREATE_H

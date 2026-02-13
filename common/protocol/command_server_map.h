#ifndef COMMAND_SERVER_MAP_H
#define COMMAND_SERVER_MAP_H

#include "./protocol_message.h"
#include "./command_server.h"

#include <QDataStream>
#include <QIODevice>

namespace server_protocol {

enum type_object_map: uint8_t{
    drone,
    marker
};

// Создать объект на карте:
// тип ебъекта(uint8_t), координаты(double,double), имя (QString)
class command_server_map_object_create: public command_server{
public:

    // data разбиваются на свойства команды
    command_server_map_object_create(const QByteArray& data)
    {
        // Считываем поля (при const QByteArray& - QIODevice::ReadOnly)
        QDataStream stream(data);
        stream.device()->seek(1); // минуем id команды
        stream >> type_obj >> lat >> lon;
    }

    command_server_map_object_create(type_object_map type_obj_, double Lat, double Lon):
        type_obj(type_obj_),
        lat(Lat), lon(Lon)
    { /* ... */}

    void toByteArray(QByteArray& boxForData) const override final
    {
        QDataStream stream(&boxForData, QIODevice::WriteOnly);
        stream << (uint8_t)id_command_server_map_object_create /// id команды, который не храним
               << (uint8_t)type_obj
               << (double)lat << (double)lon;
    }

    type_object_map type_object() const{
        return (type_object_map)type_obj;
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

#endif // COMMAND_SERVER_MAP_H

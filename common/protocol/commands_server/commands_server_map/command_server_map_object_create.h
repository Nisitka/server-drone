#ifndef COMMAND_SERVER_MAP_OBJECT_CREATE_H
#define COMMAND_SERVER_MAP_OBJECT_CREATE_H

#include <QDataStream>
#include <QIODevice>

#include "../command_server.h"
#include "./command_server_map.h"

namespace server_protocol {

// Создать объект на карте:
// тип ебъекта(uint8_t), координаты(double,double), имя (QString)
class command_server_map_object_create: public command_server{
public:

    // data разбиваются на свойства команды
    command_server_map_object_create(const QByteArray& data):
        command_server(id_command_server_map_object_create)
    {
        // Считываем поля (при const QByteArray& - QIODevice::ReadOnly)
        QDataStream stream(data);
        stream.device()->seek(1); // минуем id команды
        stream >> type_obj >> lat >> lon;
    }

    command_server_map_object_create(type_object_map type_obj_, double Lat, double Lon):
        command_server(id_command_server_map_object_create),
        type_obj(type_obj_),
        lat(Lat), lon(Lon)
    { /* ... */}

    void toByteArray(QByteArray& boxForData) const override final
    {
        QDataStream stream(&boxForData, QIODevice::WriteOnly);
        stream << (uint8_t)id_cmd
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

#endif // COMMAND_SERVER_MAP_OBJECT_CREATE_H

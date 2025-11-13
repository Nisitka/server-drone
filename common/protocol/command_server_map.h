#ifndef COMMAND_SERVER_MAP_H
#define COMMAND_SERVER_MAP_H

#include "./protocol_message.h"

#include <QDataStream>

namespace server_protocol {

// Какие есть команды для сервера
enum id_command_server_map: uint8_t{
    id_command_server_map_object_create,
    id_command_server_map_object_remove,
    id_command_server_map_object_set_position,
    id_command_server_map_requreq_objects
};

class command_server_map{
public:

    // Узнать какая команда связанная
    // с картой сервера заложена в сообщение
    static id_command_server_map get_command_id(const QByteArray& data) {
        // id команды лежит в самом начале
        return (id_command_server_map)static_cast<uint8_t>(data[0]);
    }
};

enum type_object_map: uint8_t{
    drone,
    marker
};

// Создать объект на карте:
// тип ебъекта(uint8_t), координаты(double,double), имя (QString)
class command_server_map_object_create{
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
        type_obj(type_obj_), lat(Lat), lon(Lon)
    { /* ... */}

    void toByteArray(QByteArray& boxForData) const
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

//class command_server_map: public protocol_message{
//public:
//    // Конструктор перемещения
//    command_server_map(protocol_message&& msg){
//        // Перемещаем свойства базового класса
//        *static_cast<protocol_message*>(this) = std::move(msg);
//    }

//    // Узнать какая команда связанная
//    // с картой сервера заложена в сообщение
//    id_command_server_map id() const{
//        // id команды лежит в самом начале
//        return (id_command_server_map)static_cast<uint8_t>(data[0]);
//    }
//};

#endif // COMMAND_SERVER_MAP_H

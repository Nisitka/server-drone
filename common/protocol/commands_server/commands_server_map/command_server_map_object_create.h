#ifndef COMMAND_SERVER_MAP_OBJECT_CREATE_H
#define COMMAND_SERVER_MAP_OBJECT_CREATE_H

#include <QDataStream>
#include <QIODevice>

#include <QColor>

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"

#include "./command_server_map.h"

namespace server_protocol {

// Создать объект на карте:
class command_server_map_object_create: public protocol_message,
                                        public command{
public:

    // data разбиваются на свойства команды
    command_server_map_object_create(const QByteArray& data):
        protocol_message(id_msg_command_server),
        command(id_command_server_map_object_create)
    {
        const char* dataPtr = data.constData();
        int posData = sizeof(uint8_t)*2; // минуем id_msg, id_cmd

        // uuid
        readStringFromByteArray(data, posData, posData);

        // Тип метки
        memcpy(&type_obj_id, dataPtr + posData, sizeof(uint8_t));
        posData += sizeof(uint8_t);
        memcpy(&subtype_obj_id, dataPtr + posData, sizeof(uint8_t));
        posData += sizeof(uint8_t);

        // Координаты
        memcpy(&lat, dataPtr + posData, sizeof(lat));
        posData += sizeof(lat);
        memcpy(&lon, dataPtr + posData, sizeof(lon));
        posData += sizeof(lon);

        // Имя
        name = readStringFromByteArray(data, posData, posData);
        memcpy(&color_name_r, dataPtr + posData, sizeof(uint8_t));
        posData += sizeof(uint8_t);
        memcpy(&color_name_g, dataPtr + posData, sizeof(uint8_t));
        posData += sizeof(uint8_t);
        memcpy(&color_name_b, dataPtr + posData, sizeof(uint8_t));
        posData += sizeof(uint8_t);

        // Информация
        info = readStringFromByteArray(data, posData, posData);
    }

    command_server_map_object_create(const QString& uuid_,
                                    uint8_t type_obj_id_,
                                    uint8_t subtype_obj_id_,
                                    const QString& name_,
                                    const QColor& color_name,
                                    double Lat, double Lon,
                                    const QString& info_):
        protocol_message(id_msg_command_server),
        command(id_command_server_map_object_create),
        uuid(uuid_),
        type_obj_id(type_obj_id_),
        subtype_obj_id(subtype_obj_id_),
        lat(Lat), lon(Lon),
        name(name_), info(info_),
        color_name_r(color_name.red()),
        color_name_g(color_name.green()),
        color_name_b(color_name.blue())
    { /* ... */}

    QByteArray toByteArray() const override final{
        QByteArray byteArray;

        byteArray.append(static_cast<char>(id_msg));
        byteArray.append(static_cast<char>(id_cmd));

        // uuid
        appendStringToByteArray(uuid, byteArray);

        // Тип метки
        byteArray.append(static_cast<char>(type_obj_id));
        byteArray.append(static_cast<char>(subtype_obj_id));

        // Координаты
        byteArray.append(reinterpret_cast<const char*>(&lat), sizeof(lat));
        byteArray.append(reinterpret_cast<const char*>(&lon), sizeof(lon));

        // Имя
        appendStringToByteArray(name, byteArray);
        byteArray.append(static_cast<char>(color_name_r));
        byteArray.append(static_cast<char>(color_name_g));
        byteArray.append(static_cast<char>(color_name_b));

        // Доп. информация
        appendStringToByteArray(info, byteArray);

        return byteArray;
    }

    QString UUID() const{
        return uuid;
    }

    uint8_t type_object() const{
        return type_obj_id;
    }
    uint8_t subtype_object() const{
        return subtype_obj_id;
    }

    QString Name() const{
        return name;
    }

    QString Info() const{
        return info;
    }

    QColor ColorName() const{
        return QColor(color_name_r,
                      color_name_g,
                      color_name_b);
    }
    uint8_t ColorName_R() const{
        return color_name_r;
    }
    uint8_t ColorName_G() const{
        return color_name_g;
    }
    uint8_t ColorName_B() const{
        return color_name_b;
    }

    double Lat() const{
        return lat;
    }
    double Lon() const{
        return lon;
    }

private:
    // uuid
    QString uuid;

    // Тип метки
    uint8_t type_obj_id;
    uint8_t subtype_obj_id;

    // Координаты
    double lat;
    double lon;

    // Имя
    QString name;

    // Цвет имени
    uint8_t color_name_r;
    uint8_t color_name_g;
    uint8_t color_name_b;

    // Доп. инфа
    QString info;
};

}

#endif // COMMAND_SERVER_MAP_OBJECT_CREATE_H

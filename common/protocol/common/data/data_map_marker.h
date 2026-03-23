#ifndef DATA_MAP_MARKER_H
#define DATA_MAP_MARKER_H

#include <QString>
#include <QColor>
#include <QDateTime>

#include "../../protocol_message.h"

namespace server_protocol {

class data_map_marker{
public:
    data_map_marker(const QString& uuid_,
                    uint8_t type_obj_id_,
                    uint8_t subtype_obj_id_,
                    const QString& name_,
                    const QColor& color_name,
                    double Lat, double Lon,
                    const QString& info_,
                    const QDateTime& lastUpdate_):
        isEmpty(false),
        lastUpdate(lastUpdate_),
        uuid(uuid_),
        type_obj_id(type_obj_id_),
        subtype_obj_id(subtype_obj_id_),
        lat(Lat), lon(Lon),
        colorName(color_name)
    {   }

    data_map_marker(const QByteArray& data, int posData):
        isEmpty(false),

        // uuid
        uuid(readStringFromByteArray(data, posData, posData)){

        // Дата и время последнего обновления
        lastUpdate = QDateTime::fromString(readStringFromByteArray(data, posData, posData),
                                           format_lastUpdate);

        // Тип метки
        const char* dataPtr = data.constData();
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

        // Цвет имени
        uint8_t r, g, b;
        memcpy(&r, dataPtr + posData, sizeof(uint8_t));
        posData += sizeof(uint8_t);
        memcpy(&g, dataPtr + posData, sizeof(uint8_t));
        posData += sizeof(uint8_t);
        memcpy(&b, dataPtr + posData, sizeof(uint8_t));
        posData += sizeof(uint8_t);
        colorName = QColor(r,g,b);

        // Информация
        info = readStringFromByteArray(data, posData, posData);
    }

    data_map_marker():
        isEmpty(true)
    {   }

    void appendToByteArray(QByteArray& byteArray) const{

        // uuid
        appendStringToByteArray(uuid, byteArray);

        // Дата и время последнего обновления
        appendStringToByteArray(lastUpdate.toString(format_lastUpdate),
                                byteArray);

        // Тип метки
        byteArray.append(static_cast<char>(type_obj_id));
        byteArray.append(static_cast<char>(subtype_obj_id));

        // Координаты
        byteArray.append(reinterpret_cast<const char*>(&lat), sizeof(lat));
        byteArray.append(reinterpret_cast<const char*>(&lon), sizeof(lon));

        // Имя
        appendStringToByteArray(name, byteArray);

        // Цвет имени
        byteArray.append(static_cast<char>((uint8_t)colorName.red()));
        byteArray.append(static_cast<char>((uint8_t)colorName.green()));
        byteArray.append(static_cast<char>((uint8_t)colorName.blue()));

        // Доп. информация
        appendStringToByteArray(info, byteArray);
    }

    // Пустая ли структура
    bool const isEmpty;

    // uuid
    const QString uuid;

    // Дата и время последнего обновления
    QDateTime lastUpdate;
    static const QString format_lastUpdate;

    // Тип метки
    uint8_t type_obj_id;
    uint8_t subtype_obj_id;

    // Координаты
    double lat;
    double lon;

    // Имя
    QString name;

    // Цвет имени
    QColor colorName;

    // Доп. инфа
    QString info;
};

QString const data_map_marker::format_lastUpdate = "yyyy-MM-dd HH:mm:ss";

}



#endif // DATA_MAP_MARKER_H

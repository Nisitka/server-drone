#ifndef DATA_MAP_MARKER_H
#define DATA_MAP_MARKER_H

#include <QString>
#include <QColor>
#include <QDateTime>

#include <QByteArray>
#include <QDateTime>
#include <QIODevice>
#include <QtEndian>

#include "../../protocol_message.h"

namespace server_protocol {

class data_map_marker {
public:
    data_map_marker(const QString& uuid_,
                    uint8_t type_obj_id_,
                    uint8_t subtype_obj_id_,
                    const QString& name_,
                    const QColor& color_name,
                    double Lat, double Lon,
                    const QString& info_,
                    const QDateTime& lastUpdate_) :
        m_isEmpty(false),
        lastUpdate(lastUpdate_),
        uuid(uuid_),
        type_obj_id(type_obj_id_),
        subtype_obj_id(subtype_obj_id_),
        lat(Lat), lon(Lon),
        name(name_),
        colorName(color_name),
        info(info_)
    { }

    // Конструктор десериализации (Извлекает данные из массива байт начиная с offset)
    data_map_marker(const QByteArray& data, int& offset) :
        m_isEmpty(true) // По умолчанию считаем пустой, пока полностью не распарсим
    {
        // Извлекаем UUID
        uuid = readStringFromByteArray(data, offset);

        // Дата и время последнего обновления
        QString dtStr = readStringFromByteArray(data, offset);
        lastUpdate = QDateTime::fromString(dtStr, format_lastUpdate);

        // Проверяем, хватает ли оставшихся байт на фиксированные поля:
        // 2 байта (типы) + 16 байт (2 * double) + 3 байта (RGB цвет) = 21 байт минимум
        if (offset + 21 > data.size()) {
            qWarning() << "data_map_marker: Недостаточно байт для чтения заголовка маркера!";
            return;
        }

        const char* dataPtr = data.constData();

        // Тип и подтип метки
        type_obj_id = static_cast<uint8_t>(*(dataPtr + offset));
        offset += 1;
        subtype_obj_id = static_cast<uint8_t>(*(dataPtr + offset));
        offset += 1;

        // Координаты (Безопасное чтение double с учетом сетевого порядка байт через QDataStream)
        QByteArray geoBytes = data.mid(offset, sizeof(double) * 2);
        QDataStream geoStream(geoBytes);
        geoStream >> lat;
        geoStream >> lon;
        offset += sizeof(double) * 2;

        // Имя
        name = readStringFromByteArray(data, offset);

        // Снова проверяем остаток перед чтением цвета (3 байта)
        if (offset + 3 > data.size()) {
            qWarning() << "data_map_marker: Недостаточно байт для чтения цвета!";
            return;
        }

        // Цвет имени
        uint8_t r = static_cast<uint8_t>(*(dataPtr + offset)); offset++;
        uint8_t g = static_cast<uint8_t>(*(dataPtr + offset)); offset++;
        uint8_t b = static_cast<uint8_t>(*(dataPtr + offset)); offset++;
        colorName = QColor(r, g, b);

        // Информация
        info = readStringFromByteArray(data, offset);

        // Если дошли сюда и не упали по проверкам — маркер успешно загружен
        m_isEmpty = false;
    }

    data_map_marker(): m_isEmpty(true) {}

    void appendToByteArray(QByteArray& byteArray) const {
        // uuid
        appendStringToByteArray(uuid, byteArray);

        // Дата и время последнего обновления
        appendStringToByteArray(lastUpdate.toString(format_lastUpdate), byteArray);

        // Тип метки
        byteArray.append(static_cast<char>(type_obj_id));
        byteArray.append(static_cast<char>(subtype_obj_id));

        // Координаты (Запись через QDataStream гарантирует Big-Endian формат для double)
        QByteArray geoBytes;
        QDataStream geoStream(&geoBytes, QIODevice::WriteOnly);
        geoStream << lat;
        geoStream << lon;
        byteArray.append(geoBytes);

        // Имя
        appendStringToByteArray(name, byteArray);

        // Цвет имени
        byteArray.append(static_cast<char>(static_cast<uint8_t>(colorName.red())));
        byteArray.append(static_cast<char>(static_cast<uint8_t>(colorName.green())));
        byteArray.append(static_cast<char>(static_cast<uint8_t>(colorName.blue())));

        // Доп. информация
        appendStringToByteArray(info, byteArray);
    }

    // Убираем const, чтобы класс можно было копировать и перезаписывать в контейнерах типа QMap/QVector
    bool isEmpty() const { return m_isEmpty; }

    QString get_uuid() const { return uuid; }

    QDateTime lastUpdate;
    static const QString format_lastUpdate;

    uint8_t type_obj_id = 0;
    uint8_t subtype_obj_id = 0;

    double lat = 0.0;
    double lon = 0.0;

    QString name;
    QColor colorName;
    QString info;

private:
    bool m_isEmpty;
    QString uuid; // UUID не должен меняться
};

inline const QString data_map_marker::format_lastUpdate = "yyyy-MM-dd HH:mm:ss.zzz";

}



#endif // DATA_MAP_MARKER_H

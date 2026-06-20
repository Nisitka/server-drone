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

#include <QColor>
#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QtEndian> /// qFromBigEndian
#include <cstring>

namespace server_protocol {

class data_map_marker {
public:
    data_map_marker(const QString& uuid_,
                    const QList<uint8_t>& hierarchy_chain_, // Изменено на uint8_t
                    const QString& name_,
                    const QColor& color_name,
                    double Lon, double Lat,
                    const QString& info_,
                    const QDateTime& lastUpdate_) :
        m_isEmpty(false),
        lastUpdate(lastUpdate_),
        uuid(uuid_),
        hierarchy_chain(hierarchy_chain_),
        lat(Lat), lon(Lon),
        name(name_),
        colorName(color_name),
        info(info_)
    { }

    // Конструктор десериализации (Извлекает данные из массива байт начиная с offset)
    data_map_marker(const QByteArray& data, int& offset) :
        m_isEmpty(true),
        lat(0.0),
        lon(0.0)
    {
        // UUID строки
        uuid = readStringFromByteArray(data, offset);

        // Дата обновления
        QString dtStr = readStringFromByteArray(data, offset);
        lastUpdate = QDateTime::fromString(dtStr, format_lastUpdate);

        // Читаем размер цепочки иерархии (2 байта)
        if (offset + 2 > data.size()) {
            qWarning() << "data_map_marker: Недостаточно байт для чтения размера цепочки!";
            return;
        }
        uint16_t rawChainSize;
        std::memcpy(&rawChainSize, data.constData() + offset, sizeof(uint16_t));
        offset += 2;
        uint16_t chainSize = qFromBigEndian(rawChainSize);

        // Читаем саму цепочку (по 1 байту на элемент)
        if (offset + chainSize > data.size()) {
            qWarning() << "data_map_marker: Пакет урезан, невозможно считать цепочку иерархии! Требуется байт:" << chainSize << "Осталось в пакете:" << (data.size() - offset);
            return;
        }

        hierarchy_chain.clear();
        hierarchy_chain.reserve(chainSize);
        for (int i = 0; i < chainSize; ++i) {
            // Строго используем .at() для предотвращения неявного копирования массива в Qt
            hierarchy_chain.append(static_cast<uint8_t>(data.at(offset)));
            offset += 1;
        }

        // Проверяем остаток под double координаты и цвет (8 + 8 + 3 = 19 байт)
        if (offset + 19 > data.size()) {
            qWarning() << "data_map_marker: Недостаточно байт для координат и цвета!";
            return;
        }

        double rawLon = 0.0;
        double rawLat = 0.0;

        // Читаем 8 байт долготы
        std::memcpy(&rawLon, data.constData() + offset, sizeof(double));
        offset += sizeof(double);

        // Читаем 8 байт широты
        std::memcpy(&rawLat, data.constData() + offset, sizeof(double));
        offset += sizeof(double);

        // Переводим из сетевого формата в формат текущего процессора
        lon = qFromBigEndian(rawLon);
        lat = qFromBigEndian(rawLat);

        // Имя
        name = readStringFromByteArray(data, offset);

        // Проверяем остаток под цвет (3 байта)
        if (offset + 3 > data.size()) {
            qWarning() << "data_map_marker: Недостаточно байт для цвета!";
            return;
        }

        // Читаем RGB компоненты цвета
        uint8_t r = static_cast<uint8_t>(data.at(offset)); offset++;
        uint8_t g = static_cast<uint8_t>(data.at(offset)); offset++;
        uint8_t b = static_cast<uint8_t>(data.at(offset)); offset++;
        colorName = QColor(r, g, b);

        // Дополнительная информация
        info = readStringFromByteArray(data, offset);

        // Маркер успешно и полностью распарсен
        m_isEmpty = false;
    }

    data_map_marker() : m_isEmpty(true), lat(0.0), lon(0.0) {}

    void appendToByteArray(QByteArray& byteArray) const {
        // uuid
        appendStringToByteArray(uuid, byteArray);

        // Дата и время последнего обновления
        appendStringToByteArray(lastUpdate.toString(format_lastUpdate), byteArray);

        // Сериализация цепочки принадлежности (иерархии)
        qDebug() << "data_map_marker::appendToByteArray - hierarchy_chain" << hierarchy_chain;
        if (hierarchy_chain.size() > std::numeric_limits<uint16_t>::max()) {
            qCritical() << "data_map_marker: Цепочка принадлежности слишком длинная для uint16_t!";
            return;
        }
        uint16_t chainSize = static_cast<uint16_t>(hierarchy_chain.size());
        uint16_t networkChainSize = qToBigEndian(chainSize);
        byteArray.append(reinterpret_cast<const char*>(&networkChainSize), sizeof(uint16_t));

        // Пишем каждый элемент цепочки (по 1 байту на каждый ID)
        for (uint8_t id: hierarchy_chain) {
            byteArray.append(static_cast<char>(id));
        }

        // Координаты (Сериализация double в Big-Endian)
        // Прямая упаковка double в сетевой формат
        double netLon = qToBigEndian(lon);
        double netLat = qToBigEndian(lat);
        byteArray.append(reinterpret_cast<const char*>(&netLon), sizeof(double));
        byteArray.append(reinterpret_cast<const char*>(&netLat), sizeof(double));

        // Имя
        appendStringToByteArray(name, byteArray);

        // Цвет имени
        byteArray.append(static_cast<char>(static_cast<uint8_t>(colorName.red())));
        byteArray.append(static_cast<char>(static_cast<uint8_t>(colorName.green())));
        byteArray.append(static_cast<char>(static_cast<uint8_t>(colorName.blue())));

        // Доп. информация
        appendStringToByteArray(info, byteArray);
    }

    bool isEmpty() const { return m_isEmpty; }
    QString get_uuid() const { return uuid; }

    // Цепочка вложенности типов
    static QList<uint8_t> parseHierarchyString(const QString& hierarchyStr);
    static QString makeHierarchyString(const QList<uint8_t>& chain);
    QList<uint8_t> getHierarchyChain_list() const { return hierarchy_chain; }
    void setHierarchyChain(const QList<uint8_t>& hierarchy_chain_) { hierarchy_chain = hierarchy_chain_;}
    QString getHierarchyChain_str() const {return makeHierarchyString(hierarchy_chain); }

    QDateTime lastUpdate;
    static const QString format_lastUpdate;

    // Координаты
    double lat;
    double lon;

    QString name;
    QColor colorName;
    QString info;

private:
    bool m_isEmpty;
    QString uuid;

    // Цепочка вложенности, где каждый уровень кодируется 1 байтом (0..255)
    QList<uint8_t> hierarchy_chain;
};

inline const QString data_map_marker::format_lastUpdate = "yyyy-MM-dd HH:mm:ss.zzz";

/**
 * @brief Преобразует строку вида "id_0-id_1-...-id_n" в список uint8_t
    * @param hierarchyStr Строка из базы данных
    * @return Набор байт иерархии. Если строка битая, вернет пустой список или пропустит некорректный элемент. */
inline QList<uint8_t> data_map_marker::parseHierarchyString(const QString& hierarchyStr) {
    QList<uint8_t> chain;

    if (hierarchyStr.isEmpty()) {
        return chain;
    }

    // Разбиваем строку по дефису
    // Вариант для Qt 6 (работает без выделения памяти в куче через QStringView)
    const auto parts = QStringView(hierarchyStr).split('-', Qt::SkipEmptyParts);
    chain.reserve(parts.size());

    for (const auto& part : parts) {
        bool ok = false;
        // Конвертируем в int, чтобы проверить границы uint8_t (0..255)
        int id = part.toInt(&ok);

        if (!ok) {
            qWarning() << "parseHierarchyString: Ошибка конвертации элемента строки в число:" << part;
            continue; // Или return QList<uint8_t>(); если критично прервать весь пакет
        }

        if (id < 0 || id > 255) {
            qWarning() << "parseHierarchyString: ID выходит за границы uint8_t (0-255):" << id;
            continue;
        }

        chain.append(static_cast<uint8_t>(id));
    }

    return chain;
}

/**
 * @brief Обратная функция: преобразует цепочку обратно в строку для сохранения в БД
 */
inline QString data_map_marker::makeHierarchyString(const QList<uint8_t>& chain) {
    if (chain.isEmpty()) return QString();

    QStringList parts;
    parts.reserve(chain.size());
    for (uint8_t id : chain) {
        parts.append(QString::number(id));
    }

    return parts.join('-');
}


} // namespace server_protocol

#endif // DATA_MAP_MARKER_H

#ifndef COMMAND_CLIENT_MAP_RESULT_REQUREQ_TYPE_MARKERS_H
#define COMMAND_CLIENT_MAP_RESULT_REQUREQ_TYPE_MARKERS_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../command_client.h"
#include "../../common/data/data_map_marker.h" // Нужен для appendStringToByteArray / readStringFromByteArray

namespace server_protocol {

class command_client_map_result_requreq_type_markers : public protocol_message,
                                                       public command {
public:
    // Полноценная структура записи типа для удобного обмена внутри C++ и Kotlin
    struct TypeRecord {
        QList<uint8_t> hierarchy_chain; // цепочка типов
        QString name;
        QByteArray iconBytes; // Исходные бинарные данные файла иконки (PNG или SVG)
    };

    /// ПРИЕМ НА КЛИЕНТЕ (Десериализация)
    explicit command_client_map_result_requreq_type_markers(const QByteArray& bodyData) :
        protocol_message(id_msg_command_client),
        command(id_command_client_map_result_requreq_type_markers),
        result(invalid),
        count_type_markers(0),
        snapshot(),
        m_isValid(false)
    {
        this->data = bodyData;
        int offset = 0;
        int totalSize = data.size();

        // Пропускаем id_cmd (1 байт)
        if (offset + 1 <= totalSize) {
            offset += 1;
        }

        // Безопасно считываем 1 байт результата (result)
        if (offset + 1 <= totalSize) {
            result = static_cast<results_requreq>(data.at(offset));
            offset += 1;
        } else {
            qWarning() << "command_client_map_result_requreq_type_markers: Not enough bytes to read 'result'!";
            return;
        }

        // Если сервер ответил неудачей, прерываем чтение, пакет пустой (это нормально)
        if (result == invalid) {
            m_isValid = true; // Пакет нормальный, но данных в нём нет
            return;
        }

        /// Кол-во типов (2 байта, Big-Endian)
        if (offset + 2 > totalSize) {
            qWarning() << "command_client_map_result_requreq_type_markers: Not enough bytes to read 'count_type_markers'!";
            return;
        }
        uint16_t rawCount;
        std::memcpy(&rawCount, data.constData() + offset, sizeof(uint16_t));
        offset += 2;
        count_type_markers = qFromBigEndian(rawCount);

        // Дата и время снимка данных
        const QString dtStr = readStringFromByteArray(data, offset);
        snapshot = QDateTime::fromString(dtStr, data_map_marker::format_lastUpdate);

        if (!snapshot.isValid()) {
            qWarning() << "command_client_map_result_requreq_type_markers: Invalid snapshot QDateTime format!";
            return;
        }

        type_markers_list.reserve(count_type_markers);

        // Цикл парсинга динамического массива типов меток
        for (uint16_t i = 0; i < count_type_markers; ++i) {

            // Читаем размер цепочки иерархии (2 байта, Big-Endian)
            if (offset + 2 > totalSize) return;
            uint16_t rawChainSize;
            std::memcpy(&rawChainSize, data.constData() + offset, sizeof(uint16_t));
            offset += 2;
            uint16_t chainSize = qFromBigEndian(rawChainSize);

            // Читаем саму цепочку ID (по 1 байту на каждый ID)
            if (offset + chainSize > totalSize) return;
            QList<uint8_t> chain;
            chain.reserve(chainSize); // сразу выделяем место для скорости
            for (uint16_t j = 0; j < chainSize; ++j) {
                chain.append(static_cast<uint8_t>(data.at(offset)));
                offset += 1;
            }

            // Читаем имя типа (сдвигает offset)
            QString name = readStringFromByteArray(data, offset);

            // Читаем размер бинарных данных изображения (4 байта, Big-Endian)
            if (offset + 4 > totalSize) return;
            uint32_t rawIconSize;
            std::memcpy(&rawIconSize, data.constData() + offset, sizeof(uint32_t));
            offset += 4;
            uint32_t iconSize = qFromBigEndian(rawIconSize);

            // Выделяем сырые байты картинки
            if (offset + iconSize > totalSize) return;

            TypeRecord record;
            record.hierarchy_chain = chain;
            record.name = name;
            record.iconBytes = data.mid(offset, iconSize);
            offset += iconSize;

            type_markers_list.append(record);
        }

        m_isValid = true;
    }

    /// ОТПРАВКА С СЕРВЕРА
    command_client_map_result_requreq_type_markers(results_requreq result_,
                                                   const QDateTime& snapshot_,
                                                   const QList<TypeRecord>& serverTypesList) :
        protocol_message(id_msg_command_client),
        command(id_command_client_map_result_requreq_type_markers),
        result(result_),
        count_type_markers(static_cast<uint16_t>(serverTypesList.size())),
        snapshot(snapshot_),
        m_isValid(true),
        type_markers_list(serverTypesList)
    {
        // Резервируем память: фиксированный заголовок + примерный объем под картинки
        data.reserve(1 + 1 + 2 + 30 + (type_markers_list.size() * 1024));

        // Добавляем идентификатор конкретной команды (1 байт)
        data.append(static_cast<char>(id_cmd));

        // Добавляем 1 байт результата запроса
        data.append(static_cast<char>(result));

        // Добавляем количество типов (2 байта, Big-Endian вместо старого 1 байта)
        uint16_t networkCount = qToBigEndian(count_type_markers);
        data.append(reinterpret_cast<const char*>(&networkCount), sizeof(uint16_t));

        // Добавляем дату и время снимка данных
        appendStringToByteArray(snapshot.toString(data_map_marker::format_lastUpdate), data);

        // Если результат неуспешный, массив типов не пишем, отправляем пустой пакет
        if (result == invalid) {
            return;
        }

        // Сериализуем каждую запись типа в бинарный поток последовательно
        for (const TypeRecord& record: type_markers_list) {

            // Пишем размер цепочки иерархии (2 байта, Big-Endian)
            uint16_t chainSize = static_cast<uint16_t>(record.hierarchy_chain.size());
            uint16_t networkChainSize = qToBigEndian(chainSize);
            data.append(reinterpret_cast<const char*>(&networkChainSize), sizeof(uint16_t));

            // Пишем саму цепочку ID (по 1 байту на элемент)
            for (uint8_t id : record.hierarchy_chain) {
                data.append(static_cast<char>(id));
            }

            // Пишем название типа (упакует 2 байта длины + текст)
            appendStringToByteArray(record.name, data);

            // Пишем размер бинарных данных картинки (4 байта, Big-Endian)
            uint32_t iconSize = static_cast<uint32_t>(record.iconBytes.size());
            uint32_t networkIconSize = qToBigEndian(iconSize); // как положено для передачи в сети
            data.append(reinterpret_cast<const char*>(&networkIconSize), sizeof(uint32_t));

            // Cырые байты файла изображения (PNG или SVG)
            data.append(record.iconBytes);
        }
    }

    virtual ~command_client_map_result_requreq_type_markers() override = default;

    // Геттеры для внешнего кода
    results_requreq getResult() const { return result; }
    uint16_t getCountTypeMarkers() const { return count_type_markers; }
    QDateTime getSnapshot() const { return snapshot; }
    QList<TypeRecord> getTypeMarkersList() const { return type_markers_list; }

    // Проверка: успешно ли распарсился пакет
    bool isValid() const { return m_isValid && snapshot.isValid(); }

private:
    results_requreq result;
    uint16_t count_type_markers; // Изменено на uint16_t для поддержки > 255 типов
    QDateTime snapshot;
    bool m_isValid;
    QList<TypeRecord> type_markers_list; // Наш динамический массив типов меток
};

} // namespace server_protocol


#endif // COMMAND_CLIENT_MAP_RESULT_REQUREQ_TYPE_MARKERS_H

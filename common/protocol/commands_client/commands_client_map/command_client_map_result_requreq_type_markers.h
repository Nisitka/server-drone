#ifndef COMMAND_CLIENT_MAP_RESULT_REQUREQ_TYPE_MARKERS_H
#define COMMAND_CLIENT_MAP_RESULT_REQUREQ_TYPE_MARKERS_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../command_client.h"
#include "../../common/data/data_map_marker.h"

namespace server_protocol {

class command_client_map_result_requreq_type_markers : public protocol_message,
                                                       public command {
public:
    enum motive: uint8_t{
        synced, forced, update
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
        } else {
            qWarning() << "command_client_map_result_requreq_type_markers: Data is empty!";
            return;
        }

        // Считываем результат запроса типов (1 байт)
        if (offset + 1 <= totalSize) {
            result = static_cast<results_requreq>(data.at(offset));
            offset += 1;
        } else {
            qWarning() << "command_client_map_result_requreq_type_markers: Not enough bytes for 'result'!";
            return;
        }
        // Если сервер ответил ошибкой, прерываем чтение
        if (result == invalid) {
            m_isValid = true;
            return;
        }

        // Из-за чего отправлено это сообщение клиенту
        if (offset + 1 <= totalSize) {
            motive_value = static_cast<motive>(data.at(offset));
            offset += 1;
        } else {
            qWarning() << "command_client_map_result_requreq_type_markers: Not enough bytes for 'motive'!";
            return;
        }

        // Читаем дату и время снимка данных
        const QString dtStr = readStringFromByteArray(data, offset);
        snapshot = QDateTime::fromString(dtStr, data_map_marker::format_lastUpdate());
        if (!snapshot.isValid()) return;

        // Читаем количество ЖИВЫХ типов (2 байта, Big-Endian)
        if (offset + 2 > totalSize) return;
        uint16_t rawLiveCount;
        std::memcpy(&rawLiveCount, data.constData() + offset, sizeof(uint16_t));
        offset += 2;
        count_type_markers = qFromBigEndian(rawLiveCount);

        // Читаем количество УДАЛЕННЫХ цепочек (2 байта, Big-Endian)
        if (offset + 2 > totalSize) return;
        uint16_t rawDeletedCount;
        std::memcpy(&rawDeletedCount, data.constData() + offset, sizeof(uint16_t));
        offset += 2;
        uint16_t deletedCount = qFromBigEndian(rawDeletedCount);

        // Цикл парсинга удаленных цепочек
        deleted_chains_list.reserve(deletedCount);
        for (uint16_t i = 0; i < deletedCount; ++i) {
            if (offset + 2 > totalSize) return;
            uint16_t rawChainSize;
            std::memcpy(&rawChainSize, data.constData() + offset, sizeof(uint16_t));
            offset += 2;
            uint16_t chainSize = qFromBigEndian(rawChainSize);

            if (offset + chainSize > totalSize) return;
            QList<uint8_t> chain;
            chain.reserve(chainSize);
            for (uint16_t j = 0; j < chainSize; ++j) {
                chain.append(static_cast<uint8_t>(data.at(offset)));
                offset += 1;
            }
            deleted_chains_list.append(chain);
        }

        // Парсинг живых типов
        type_markers_list.reserve(count_type_markers);
        for (uint16_t i = 0; i < count_type_markers; ++i) {
            // Вызываем конструктор десериализации структуры, она сама сдвинет offset
            data_type_marker_record record(data, offset);
            type_markers_list.append(record);
        }

        m_isValid = true;
    }

    /// ОТПРАВКА С СЕРВЕРА (Сериализация ответа)
    command_client_map_result_requreq_type_markers(results_requreq result_,
                                                   motive motive_,
                                                   const QDateTime& snapshot_,
                                                   const QList<data_type_marker_record>& liveTypesList,
                                                   const QList<QList<uint8_t>>& deletedChainsList):
        protocol_message(id_msg_command_client),
        command(id_command_client_map_result_requreq_type_markers),
        result(result_),
        motive_value(motive_),
        count_type_markers(static_cast<uint16_t>(liveTypesList.size())),
        snapshot(snapshot_),
        m_isValid(true),
        type_markers_list(liveTypesList),
        deleted_chains_list(deletedChainsList)
    {
        data.reserve(1 + 1 + 2 + 30 + 2 + (deleted_chains_list.size() * 4) + (type_markers_list.size() * 1024));

        // Код команды (1 байт)
        data.append(static_cast<char>(id_cmd));

        // Результат запроса (1 байт)
        data.append(static_cast<char>(result));
        if (result == invalid) {
            return;
        }

        // Из-за чего отправлено это сообщение клиенту
        data.append(static_cast<char>(motive_value));

        // НОВАЯ ПОСЛЕДОВАТЕЛЬНОСТЬ: Дата и время снимка данных по сети идет сразу за результатом
        appendStringToByteArray(snapshot.toString(data_map_marker::format_lastUpdate()), data);

        // Количество ЖИВЫХ типов (2 байта, Big-Endian)
        uint16_t networkLiveCount = qToBigEndian(count_type_markers);
        data.append(reinterpret_cast<const char*>(&networkLiveCount), sizeof(uint16_t));

        // Количество УДАЛЕННЫХ цепочек (2 байта, Big-Endian)
        uint16_t deletedCount = static_cast<uint16_t>(deleted_chains_list.size());
        uint16_t networkDeletedCount = qToBigEndian(deletedCount);
        data.append(reinterpret_cast<const char*>(&networkDeletedCount), sizeof(uint16_t));

        // Сериализация УДАЛЕННЫХ цепочек
        for (const QList<uint8_t>& chain: deleted_chains_list) {
            uint16_t chainSize = static_cast<uint16_t>(chain.size());
            uint16_t networkChainSize = qToBigEndian(chainSize);
            data.append(reinterpret_cast<const char*>(&networkChainSize), sizeof(uint16_t));

            for (uint8_t id: chain) {
                data.append(static_cast<char>(id));
            }
        }

        // Cериализация ЖИВЫХ типов
        for (const data_type_marker_record& record: type_markers_list) {
            data.append(record.toByteArray());
        }
    }

    virtual ~command_client_map_result_requreq_type_markers() override = default;

    // Геттеры
    results_requreq getResult() const { return result; }
    uint16_t getCountTypeMarkers() const { return count_type_markers; }
    QDateTime getSnapshot() const { return snapshot; }
    QList<data_type_marker_record> getTypeMarkersList() const { return type_markers_list; }
    QList<QList<uint8_t>> getDeletedChainsList() const { return deleted_chains_list; }
    bool isValid() const { return m_isValid && snapshot.isValid(); }
    motive getMotive() const { return motive_value; };

private:
    results_requreq result;
    motive motive_value;
    uint16_t count_type_markers;
    QDateTime snapshot;
    bool m_isValid;

    QList<data_type_marker_record> type_markers_list;
    QList<QList<uint8_t>> deleted_chains_list;
};

} // namespace server_protocol


#endif // COMMAND_CLIENT_MAP_RESULT_REQUREQ_TYPE_MARKERS_H

#ifndef COMMAND_SERVER_MAP_RESULT_REQUREQ_TYPE_MARKERS_H
#define COMMAND_SERVER_MAP_RESULT_REQUREQ_TYPE_MARKERS_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"

#include "../../common/data/data_map_marker.h"

namespace server_protocol {

/// Ответ серверу что клиент получил (или не смог получить) типы меток
class command_server_map_result_requreq_type_markers:   public protocol_message,
                                                        public command {

public:
    command_server_map_result_requreq_type_markers(const QByteArray& bodyData):
        protocol_message(id_msg_command_server),
    command(id_command_server_map_result_requreq_type_markers)
    {
        // Сохраняем пришедшие байты в базовый класс
        this->data = bodyData;

        int offset = 0;

        // Пропускаем id_cmd (1 байт), так как он идет первым в теле команды
        if (offset + sizeof(uint8_t) <= static_cast<size_t>(data.size())) {
            offset += sizeof(uint8_t);
        }

        /// Результат
        if (offset + sizeof(uint8_t) <= static_cast<size_t>(data.size())) {
            result = static_cast<results_requreq>(data[offset]);
            offset += sizeof(uint8_t);
        }

        // Дата и время снимка данных
        const QString dtStr = readStringFromByteArray(data, offset);
        snapshot = QDateTime::fromString(dtStr, data_map_marker::format_lastUpdate);
    }

    command_server_map_result_requreq_type_markers(const QDateTime& snapshot_, uint8_t result_):
        protocol_message(id_msg_command_server),
        command(id_command_server_map_result_requreq_type_markers),
        snapshot(snapshot_), result(result_)
    {
        // Сначала добавляем идентификатор конкретной команды (1 байт)
        data.append(static_cast<char>(id_cmd));

        /// Результат
        data.append(static_cast<char>(result));

        // Добавляем дату и время снимка данных (строки запишутся с 2 байтами длины в Big-Endian)
        appendStringToByteArray(snapshot.toString(data_map_marker::format_lastUpdate), data);
    }

    // На случай удаления через интерфейсы
    virtual ~command_server_map_result_requreq_type_markers() override = default;

    QDateTime getSnapshot() const {
        return snapshot;
    }

    results_requreq getResult() const {
        return (results_requreq)result;
    }

private:
    QDateTime snapshot;
    uint8_t result;
};

}

#endif // COMMAND_SERVER_MAP_RESULT_REQUREQ_TYPE_MARKERS_H

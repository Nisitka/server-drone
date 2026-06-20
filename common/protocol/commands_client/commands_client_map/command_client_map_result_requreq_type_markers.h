#ifndef COMMAND_CLIENT_MAP_RESULT_REQUREQ_TYPE_MARKERS_H
#define COMMAND_CLIENT_MAP_RESULT_REQUREQ_TYPE_MARKERS_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"

#include "../command_client.h"

#include "../../common/data/data_map_marker.h"

namespace server_protocol {

/// Ответ клиенту на запрос типов меток
class command_client_map_result_requreq_type_markers:   public protocol_message,
                                                        public command {
public:
    command_client_map_result_requreq_type_markers(const QByteArray& bodyData):
        protocol_message(id_msg_command_client),
        command(id_command_client_map_result_requreq_type_markers),
        result(invalid),
        count_type_markers(0),
        snapshot()
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
            qWarning() << "command_client_map_result_requreq_type_markers: There are not enough bytes to read the 'result' field!";
            return;
        }

        // Безопасно считываем 1 байт кол-ва типов (count_type_markers)
        if (offset + 1 <= totalSize) {
            count_type_markers = static_cast<uint8_t>(data.at(offset));
            offset += 1;
        } else {
            qWarning() << "command_client_map_result_requreq_type_markers: There are not enough bytes to read the 'count_type_markers' field!";
            return;
        }

        // Дата и время снимка данных
        const QString dtStr = readStringFromByteArray(data, offset);
        snapshot = QDateTime::fromString(dtStr, data_map_marker::format_lastUpdate);
    }

    command_client_map_result_requreq_type_markers(results_requreq result_, uint8_t count_type_markers_, const QDateTime& snapshot_):
        protocol_message(id_msg_command_client),
        command(id_command_client_map_result_requreq_type_markers),
        result(result_),
        count_type_markers(count_type_markers_),
        snapshot(snapshot_)
    {
        // Оптимизация: резервируем примерный объем памяти (1 + 1 + 1 + ~25 байт для даты)
        data.reserve(32);

        // Сначала добавляем идентификатор конкретной команды (1 байт)
        data.append(static_cast<char>(id_cmd));

        // Добавляем 1 байт результата запроса
        data.append(static_cast<char>(result));

        // Добавляем 1 байт кол-ва типов
        data.append(static_cast<char>(count_type_markers));

        // Добавляем дату и время снимка данных (строки запишутся с 2 байтами длины в Big-Endian)
        appendStringToByteArray(snapshot.toString(data_map_marker::format_lastUpdate), data);
    }

    // На случай удаления через интерфейсы
    virtual ~command_client_map_result_requreq_type_markers() override = default;

    // Геттеры для внешнего кода
    results_requreq getResult() const { return result; }
    uint8_t getCountTypeMarkers() const { return count_type_markers; }
    QDateTime getSnapshot() const { return snapshot; }

    // Функция проверки: успешно ли распарсился пакет
    bool isValid() const { return snapshot.isValid() && result != invalid; }

private:
    results_requreq result;     // Готов ди сервер вообще отправить их
    uint8_t count_type_markers; // Сколько ожидать типов
    QDateTime snapshot;         // По состоянию на какое время будут эти типы

};

}



#endif // COMMAND_CLIENT_MAP_RESULT_REQUREQ_TYPE_MARKERS_H

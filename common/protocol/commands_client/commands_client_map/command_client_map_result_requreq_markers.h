#ifndef COMMAND_CLIENT_MAP_RESULT_REQUREQ_MARKERS_H
#define COMMAND_CLIENT_MAP_RESULT_REQUREQ_MARKERS_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"

#include "../command_client.h"

namespace server_protocol {

class command_client_map_result_requreq_markers:    public protocol_message,
                                                    public command {
public:

    // -------------------------------------------------------------
    // Сценарий 1: ПРИЕМ НА КЛИЕНТЕ (Конструктор десериализации)
    // -------------------------------------------------------------
    // Сюда передается чистый bodyData (уже без 4 байт сетевого заголовка протокола)
    command_client_map_result_requreq_markers(const QByteArray& bodyData):
        protocol_message(id_msg_command_client),
        command(id_command_client_map_result_requreq_markers),
        result(invalid),
        count_markers(0)
    {
        // Сохраняем пришедшие байты в базовый класс
        this->data = bodyData;

        // Пропускаем id_cmd (1 байт)
        int offset = 0;
        if (offset + sizeof(uint8_t) <= static_cast<size_t>(data.size())) {
            offset += sizeof(uint8_t);
        }

        // Безопасно считываем 1 байт результата (result)
        if (offset + sizeof(uint8_t) <= static_cast<size_t>(data.size())) {
            result = static_cast<results_requreq>(data[offset]);
            offset += sizeof(uint8_t);
        } else {
            qWarning() << "command_client_map_result_requreq_markers: There are not enough bytes to read the result field!";
            return;
        }

        // Безопасно считываем 4 байта количества маркеров (Big-Endian)
        if (offset + sizeof(uint32_t) <= static_cast<size_t>(data.size())) {
            uint32_t rawCount;
            std::memcpy(&rawCount, data.constData() + offset, sizeof(uint32_t));
            count_markers = qFromBigEndian(rawCount);
        } else {
            qWarning() << "command_client_map_result_requreq_markers: There are not enough bytes to read the count_markers field!";
        }
    }

    // -------------------------------------------------------------
    // Сценарий 2: ОТПРАВКА С СЕРВЕРА (Конструктор сериализации)
    // -------------------------------------------------------------
    command_client_map_result_requreq_markers(results_requreq result_, uint32_t count_markers_) :
        protocol_message(id_msg_command_client),
        command(id_command_client_map_result_requreq_markers),
        result(result_),
        count_markers(count_markers_)
    {
        // Сначала добавляем идентификатор конкретной команды (1 байт)
        data.append(static_cast<char>(id_cmd));

        // Добавляем 1 байт результата запроса
        data.append(static_cast<char>(result));

        // Переводим количество маркеров в сетевой порядок байт (4 байта) и добавляем в буфер
        uint32_t networkCount = qToBigEndian(count_markers);
        data.append(reinterpret_cast<const char*>(&networkCount), sizeof(networkCount));
    }

    // Возвращаем строгое перечисление вместо uint8_t для удобства в switch/case
    results_requreq getResult() const {
        return result;
    }

    uint32_t getCountMarkers() const {
        return count_markers;
    }

private:
    results_requreq result;
    uint32_t count_markers;
};

}

#endif // COMMAND_CLIENT_MAP_RESULT_REQUREQ_MARKERS_H

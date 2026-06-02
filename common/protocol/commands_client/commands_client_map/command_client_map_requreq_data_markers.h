#ifndef COMMAND_CLIENT_MAP_REQUREQ_DATA_MARKERS_H
#define COMMAND_CLIENT_MAP_REQUREQ_DATA_MARKERS_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"

#include "../command_client.h"

#include "../../common/data/data_map_marker.h"

namespace server_protocol {

class command_client_map_requreq_data_markers : public protocol_message,
                                                public command {
public:
    // -------------------------------------------------------------
    // Сценарий 1: ПРИЕМ НА КЛИЕНТЕ (Конструктор десериализации)
    // -------------------------------------------------------------
    // Сюда передается чистый bodyData (уже без 4 байт сетевого заголовка протокола)
    command_client_map_requreq_data_markers(const QByteArray& bodyData) :
        protocol_message(id_msg_command_client),
        command(id_command_client_map_requreq_data_markers),
        // Инициализируем константный маркер через лямбду со смещением offset
        data_marker([&]() {
            int offset = 0;
            // Пропускаем id_cmd (1 байт)
            if (offset + sizeof(uint8_t) <= static_cast<size_t>(bodyData.size())) {
                offset += sizeof(uint8_t);
            }
            // Конструктор маркера вычитает свои поля и сам обновит offset
            return data_map_marker(bodyData, offset);
        }())
    {
        // Сохраняем пришедшие байты в базовый класс для истории или отладки
        this->data = bodyData;
    }

    // -------------------------------------------------------------
    // Сценарий 2: ОТПРАВКА С СЕРВЕРА (Конструктор сериализации)
    // -------------------------------------------------------------
    command_client_map_requreq_data_markers(const data_map_marker& marker) :
        protocol_message(id_msg_command_client),
        command(id_command_client_map_requreq_data_markers),
        data_marker(marker)
    {
        // Сначала добавляем идентификатор конкретной команды (1 байт)
        data.append(static_cast<char>(id_cmd));

        // Добавляем сериализованные данные самого маркера
        data_marker.appendToByteArray(data);
    }

    const data_map_marker& dataMarker() const {
        return data_marker;
    }

private:
    // Поле осталось константным, как в вашей исходной архитектуре
    const data_map_marker data_marker;
};

}

#endif // COMMAND_CLIENT_MAP_REQUREQ_DATA_MARKERS_H

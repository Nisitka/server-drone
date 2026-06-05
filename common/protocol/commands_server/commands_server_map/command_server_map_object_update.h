#ifndef COMMAND_SERVER_MAP_OBJECT_UPDATE_H
#define COMMAND_SERVER_MAP_OBJECT_UPDATE_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"

#include "../../common/data/data_map_marker.h"

namespace server_protocol {

// Обновить объект на карте:
class command_server_map_object_update: public protocol_message,
                                        public command {
public:

    enum id_info_result: uint8_t {
        invalid_date_or_time_changed = 1
    };

    // -------------------------------------------------------------
    // Сценарий 1: ПРИЕМ ИЗ СЕТИ (Конструктор десериализации)
    // -------------------------------------------------------------
    // Сюда передается чистый bodyData (уже без 4 байт сетевого заголовка протокола)
    command_server_map_object_update(const QByteArray& bodyData) :
        protocol_message(id_msg_command_server),
        command(id_command_server_map_object_update)
    {
        // Сохраняем пришедшие байты в базовый класс
        this->data = bodyData;

        int offset = 0;

        // Пропускаем id_cmd (1 байт), так как он идет первым в теле команды
        if (offset + sizeof(uint8_t) <= static_cast<size_t>(data.size())) {
            offset += sizeof(uint8_t);
        }

        //  Передаем массив данных и ссылку на смещение в маркер карты.
        // Конструктор маркера сам безопасно вычитает все свои поля и сдвинет offset.
        data_marker = data_map_marker(data, offset);
    }

    // -------------------------------------------------------------
    // Сценарий 2: СОЗДАНИЕ ДЛЯ ОТПРАВКИ (Конструктор сериализации)
    // -------------------------------------------------------------
    command_server_map_object_update(const data_map_marker& data_marker_) :
        protocol_message(id_msg_command_server),
        command(id_command_server_map_object_update),
        data_marker(data_marker_)
    {
        // Формируем внутреннее тело (data) для этой конкретной команды.
        // Внешний заголовок (MagicByte, id_msg, общий размер) базовый класс соберет сам!

        // Сначала добавляем идентификатор конкретной команды (1 байт)
        data.append(static_cast<char>(id_cmd));

        // Добавляем прикладные данные маркера карты
        data_marker.appendToByteArray(data);
    }

    const data_map_marker& getDataMarker() const {
        return data_marker;
    }

private:
    data_map_marker data_marker;
};

}

#endif // COMMAND_SERVER_MAP_OBJECT_UPDATE_H

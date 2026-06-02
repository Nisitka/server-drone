#ifndef COMMAND_CLIENT_MAP_OBJECT_UPDATE_H
#define COMMAND_CLIENT_MAP_OBJECT_UPDATE_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_client/command_client.h"

#include "../../common/data/data_map_marker.h"

namespace server_protocol {

class command_client_map_object_update : public protocol_message,
                                         public command {
public:
    // -------------------------------------------------------------
    // Сценарий 1: ПРИЕМ НА КЛИЕНТЕ (Конструктор десериализации)
    // -------------------------------------------------------------
    // Сюда передается чистый bodyData (уже без 4 байт сетевого заголовка протокола)
    command_client_map_object_update(const QByteArray& bodyData) :
        protocol_message(id_msg_command_client),
        command(id_command_client_map_object_update),
        // Безопасно инициализируем константный маркер через лямбду со смещением offset
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
        // Сохраняем пришедшие байты в базовый класс
        this->data = bodyData;
    }

    // -------------------------------------------------------------
    // Сценарий 2: ОТПРАВКА С СЕРВЕРА (Конструктор сериализации)
    // -------------------------------------------------------------
    command_client_map_object_update(const data_map_marker& data_) :
        protocol_message(id_msg_command_client),
        command(id_command_client_map_object_update),
        data_marker(data_)
    {
        // Сначала добавляем идентификатор конкретной команды (1 байт)
        data.append(static_cast<char>(id_cmd));

        // Добавляем прикладные данные маркера карты
        data_marker.appendToByteArray(data);
    }

    const data_map_marker& getDataMarker() const {
        return data_marker;
    }

    // Поле осталось public и const, как в вашей исходной структуре
    const data_map_marker data_marker;
};

}

#endif // COMMAND_CLIENT_MAP_OBJECT_UPDATE_H

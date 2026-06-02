#ifndef COMMAND_SERVER_MAP_REMOVE_OBJECT_H
#define COMMAND_SERVER_MAP_REMOVE_OBJECT_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"

namespace server_protocol {

class command_server_map_remove_object : public protocol_message,
                                         public command {
public:
    // -------------------------------------------------------------
    // Сценарий 1: ПРИЕМ ИЗ СЕТИ (Конструктор десериализации)
    // -------------------------------------------------------------
    // Сюда передается чистый bodyData (уже без 4 байт сетевого заголовка протокола)
    command_server_map_remove_object(const QByteArray& bodyData) :
        protocol_message(id_msg_command_server),
        command(id_command_server_map_object_remove),
        // Инициализируем константу напрямую через вызов функции со смещением.
        // Смещение равно 1, так как в начале bodyData идет только 1 байт (id_cmd)
        uuid_marker([&]() {
            int offset = sizeof(uint8_t); // Пропускаем id_cmd
            return readStringFromByteArray(bodyData, offset);
        }())
    {
        // Сохраняем пришедшие байты в базовый класс для истории/отладки
        this->data = bodyData;
    }

    // -------------------------------------------------------------
    // Сценарий 2: СОЗДАНИЕ ДЛЯ ОТПРАВКИ (Конструктор сериализации)
    // -------------------------------------------------------------
    command_server_map_remove_object(const QString& uuid) :
        protocol_message(id_msg_command_server),
        command(id_command_server_map_object_remove),
        uuid_marker(uuid)
    {
        // Формируем внутреннее тело (data) для этой конкретной команды.
        // Внешний заголовок протокола базовый класс добавит сам!

        // Сначала добавляем идентификатор конкретной команды (1 байт)
        data.append(static_cast<char>(id_cmd));

        // Добавляем UUID удаляемого маркера (запишется с 2 байтами длины в Big-Endian)
        appendStringToByteArray(uuid_marker, data);
    }

    // UUID метки
    const QString uuid_marker;
};

}

#endif // COMMAND_SERVER_MAP_REMOVE_OBJECT_H

#ifndef COMMAND_CLIENT_MAP_OBJECT_REMOVED_H
#define COMMAND_CLIENT_MAP_OBJECT_REMOVED_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_client/command_client.h"

namespace server_protocol {

class command_client_map_object_removed : public protocol_message,
                                          public command {
public:
    // -------------------------------------------------------------
    // Сценарий 1: ПРИЕМ НА КЛИЕНТЕ (Конструктор десериализации)
    // -------------------------------------------------------------
    // Сюда передается чистый bodyData (уже без 4 байт сетевого заголовка протокола)
    command_client_map_object_removed(const QByteArray& bodyData) :
        protocol_message(id_msg_command_client),
        command(id_command_client_map_object_removed),
        // Инициализируем константу напрямую через вызов лямбды со смещением.
        // Смещение равно 1, так как в начале bodyData идет только 1 байт (id_cmd)
        uuid_object([&]() {
            int offset = sizeof(uint8_t); // Пропускаем id_cmd
            return readStringFromByteArray(bodyData, offset);
        }())
    {
        // Сохраняем пришедшие байты в базовый класс
        this->data = bodyData;
    }

    // -------------------------------------------------------------
    // Сценарий 2: ОТПРАВКА С СЕРВЕРА (Конструктор сериализации)
    // -------------------------------------------------------------
    command_client_map_object_removed(const QString& uuid_object_) :
        protocol_message(id_msg_command_client),
        command(id_command_client_map_object_removed),
        uuid_object(uuid_object_)
    {
        // Сначала добавляем идентификатор конкретной команды (1 байт)
        data.append(static_cast<char>(id_cmd));

        // Добавляем UUID удаляемого объекта (2 байта длины в Big-Endian + тело строки)
        appendStringToByteArray(uuid_object, data);
    }

    // Поле осталось public и const, как в вашей исходной архитектуре
    const QString uuid_object;
};

}

#endif // COMMAND_CLIENT_MAP_OBJECT_REMOVED_H

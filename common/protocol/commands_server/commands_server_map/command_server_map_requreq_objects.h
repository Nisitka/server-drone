#ifndef COMMAND_SERVER_MAP_REQUREQ_OBJECTS_H
#define COMMAND_SERVER_MAP_REQUREQ_OBJECTS_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"

namespace server_protocol {

// Запрос объектов карты (команда без тела данных):
class command_server_map_requreq_objects:  public protocol_message,
                                           public command {
public:
    // -------------------------------------------------------------
    // Сценарий 1: ПРИЕМ ИЗ СЕТИ (Конструктор десериализации)
    // -------------------------------------------------------------
    // Сюда передается чистый bodyData (уже без 4 байт сетевого заголовка протокола)
    command_server_map_requreq_objects(const QByteArray& bodyData):
        protocol_message(id_msg_command_server),
        command(id_command_server_map_requreq_objects)
    {
        // Сохраняем пришедшие байты в базовый класс
        this->data = bodyData;

        // Так как у команды нет других данных, кроме id_cmd,
        // нам не нужно ничего парсить с помощью offset.
    }

    // -------------------------------------------------------------
    // Сценарий 2: СОЗДАНИЕ ДЛЯ ОТПРАВКИ (Конструктор сериализации)
    // -------------------------------------------------------------
    command_server_map_requreq_objects() :
        protocol_message(id_msg_command_server),
        command(id_command_server_map_requreq_objects)
    {
        // Формируем внутреннее тело (data) для этой конкретной команды.
        // Записываем только идентификатор конкретной команды (1 байт)
        data.append(static_cast<char>(id_cmd));
    }

};

}

#endif // COMMAND_SERVER_MAP_REQUREQ_OBJECTS_H

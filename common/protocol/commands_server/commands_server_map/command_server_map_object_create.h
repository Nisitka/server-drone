#ifndef COMMAND_SERVER_MAP_OBJECT_CREATE_H
#define COMMAND_SERVER_MAP_OBJECT_CREATE_H

#include <QDataStream>
#include <QIODevice>

#include <QColor>

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"

#include "../../common/data/data_map_marker.h"

namespace server_protocol {

// Создать объект на карте:
class command_server_map_object_create : public protocol_message,
                                         public command {
public:
    // -------------------------------------------------------------
    // Сценарий 1: ПРИЕМ ИЗ СЕТИ (Конструктор десериализации)
    // -------------------------------------------------------------
    // Сюда должен передаваться ЧИСТЫЙ bodyData (уже без заголовка протокола 4 байт)
    command_server_map_object_create(const QByteArray& bodyData) :
        protocol_message(id_msg_command_server),
        command(id_command_server_map_object_create)
    {
        this->data = bodyData;
        int offset = 0;

        // Пропускаем 1 байт id_cmd
        if (offset + 1 <= data.size()) {
            offset += 1;
        }

        // Передаем весь массив данных и ссылку на смещение.
        // Конструктор маркера сам считает сколько ему нужно и корректно сдвинет offset вперед.
        data_marker = data_map_marker(data, offset);
    }

    // -------------------------------------------------------------
    // Сценарий 2: СОЗДАНИЕ ДЛЯ ОТПРАВКИ (Конструктор сериализации)
    // -------------------------------------------------------------
    command_server_map_object_create(const data_map_marker& data_marker_) :
        protocol_message(id_msg_command_server),
        command(id_command_server_map_object_create),
        data_marker(data_marker_)
    {
        // Формируем ВНУТРЕННЕЕ тело (data) для этой конкретной команды.
        // Внешний заголовок протокола базовый класс добавит сам

        // Сначала добавляем идентификатор конкретной команды (1 байт)
        data.append(static_cast<char>(id_cmd));

        // Добавляем прикладные данные маркера карты внутрь нашего буфера data
        data_marker.appendToByteArray(data);
    }

    const data_map_marker& getDataMarker() const {
        return data_marker;
    }

private:
    data_map_marker data_marker;
};

}

#endif // COMMAND_SERVER_MAP_OBJECT_CREATE_H

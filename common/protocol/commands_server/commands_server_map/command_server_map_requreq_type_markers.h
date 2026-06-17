#ifndef COMMAND_SERVER_MAP_REQUREQ_TYPE_MARKERS_H
#define COMMAND_SERVER_MAP_REQUREQ_TYPE_MARKERS_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"

namespace server_protocol {

/// Запросить данные об типах меток
class command_server_map_requreq_type_markers:  public protocol_message,
                                                public command {

public:
    // Конструктор для парсинга (чтения) пришедших данных
    explicit command_server_map_requreq_type_markers(const QByteArray& bodyData) :
        protocol_message(id_msg_command_server),
        command(id_command_server_map_requreq_type_markers)
    {
        this->data = bodyData;

        // Валидация: пакет не должен быть меньше 1 байта (id_cmd)
        if (data.isEmpty()) {
            qWarning() << "command_server_map_requreq_type_markers: Received empty body data!";
        }
    }

    // Конструктор для создания пакета на отправку
    command_server_map_requreq_type_markers() :
        protocol_message(id_msg_command_server),
        command(id_command_server_map_requreq_type_markers)
    {
        // Выделяем память сразу под 1 байт
        data.reserve(1);

        // Формируем внутреннее тело (data) для этой конкретной команды.
        // Записываем только идентификатор конкретной команды (1 байт)
        data.append(static_cast<char>(id_cmd));
    }

    // Виртуальный деструктор для безопасного полиморфного удаления
    virtual ~command_server_map_requreq_type_markers() override = default;

    // Метод проверки корректности пакета
    bool isValid() const { return !data.isEmpty(); }
};

}

#endif // COMMAND_SERVER_MAP_REQUREQ_TYPE_MARKERS_H

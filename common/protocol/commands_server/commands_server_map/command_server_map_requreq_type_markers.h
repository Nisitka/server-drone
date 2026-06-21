#ifndef COMMAND_SERVER_MAP_REQUREQ_TYPE_MARKERS_H
#define COMMAND_SERVER_MAP_REQUREQ_TYPE_MARKERS_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"
#include <QDebug>

namespace server_protocol {

/// Запросить данные о типах меток
class command_server_map_requreq_type_markers : public protocol_message,
                                                public command {
public:


    /// ПРИЕМ НА СЕРВЕРЕ
    explicit command_server_map_requreq_type_markers(const QByteArray& bodyData) :
        protocol_message(id_msg_command_server),
        command(id_command_server_map_requreq_type_markers),
        isForced(false),
        m_isValid(false)
    {
        this->data = bodyData;
        int offset = 0;
        int totalSize = data.size();

        // Пропускаем id_cmd (1 байт)
        if (offset + 1 <= totalSize) {
            offset += 1;
        } else {
            qWarning() << "command_server_map_requreq_type_markers: Received empty or corrupted body data!";
            return;
        }

        // Безопасно считываем флаг isForced (1 байт)
        if (offset + 1 <= totalSize) {
            // Конвертируем байт в булевое значение (0 — false, всё остальное — true)
            isForced = static_cast<bool>(data.at(offset));
            offset += 1;
        } else {
            qWarning() << "command_server_map_requreq_type_markers: Missing 'isForced' field in packet!";
            return;
        }

        m_isValid = true;
    }

    /// ОТПРАВКА С КЛИЕНТА
    explicit command_server_map_requreq_type_markers(bool forced = false) :
        protocol_message(id_msg_command_server),
        command(id_command_server_map_requreq_type_markers),
        isForced(forced),
        m_isValid(true)
    {
        // Выделяем память сразу под 2 байта (1 байт id_cmd + 1 байт isForced)
        data.reserve(2);

        // Записываем идентификатор конкретной команды (1 байт)
        data.append(static_cast<char>(id_cmd));

        // Записываем флаг принудительного обновления (1 байт)
        data.append(static_cast<char>(isForced ? 1 : 0));
    }

    virtual ~command_server_map_requreq_type_markers() override = default;

    // Геттер для использования на стороне сервера
    bool isForcedUpdate() const { return isForced; }

    // Метод проверки корректности пакета
    bool isValid() const { return m_isValid; }

private:
    bool isForced;
    bool m_isValid;
};

} // namespace server_protocol

#endif // COMMAND_SERVER_MAP_REQUREQ_TYPE_MARKERS_H

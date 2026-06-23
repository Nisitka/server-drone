#ifndef COMMAND_SERVER_MAP_CREATE_TYPE_MARKERS_H
#define COMMAND_SERVER_MAP_CREATE_TYPE_MARKERS_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"
#include "../../common/data/data_map_marker.h"

#include <QDebug>

namespace server_protocol {

/// Создать тип меток
class command_server_map_create_type_markers:   public protocol_message,
                                                public command {
public:

    /// ПРИЕМ НА СЕРВЕРЕ
    explicit command_server_map_create_type_markers(const QByteArray& bodyData):
        protocol_message(id_msg_command_server),
        command(id_command_server_map_create_type_markers),
        type_marker(),
        m_isValid(false)
    {
        this->data = bodyData;
        int offset = 0;
        int totalSize = data.size();

        // Пропускаем id_cmd (1 байт)
        if (offset + 1 <= totalSize) {
            offset += 1;
        } else {
            qWarning() << "command_server_map_create_type_markers: Data is empty!";
            return;
        }

        // Восстанавливаем данные типа метки из байт
        type_marker = data_type_marker_record(data, offset);

        // ЗАЩИТА: Проверяем, что структура считалась полностью, а не оборвалась посреди сети
        // Валидным типом считаем тот, у которого есть хотя бы один ID в иерархии и имя
        if (type_marker.hierarchy_chain.isEmpty() || type_marker.name.isEmpty()) {
            qWarning() << "command_server_map_create_type_markers: Received corrupted or incomplete type record!";
            return;
        }

        m_isValid = true;
    }

    /// ОТПРАВКА С КЛИЕНТА
    explicit command_server_map_create_type_markers(const data_type_marker_record& type_marker_):
        protocol_message(id_msg_command_server),
        command(id_command_server_map_create_type_markers),
        type_marker(type_marker_),
        m_isValid(true)
    {
        // Оптимизация: резервируем память под 1 байт команды + размер упакованной структуры
        QByteArray serializedRecord = type_marker.toByteArray();
        data.reserve(1 + serializedRecord.size());

        // Код команды (1 байт)
        data.append(static_cast<char>(id_cmd));

        // Данные типа метки
        data.append(serializedRecord);
    }

    data_type_marker_record get_type_marker() const {return type_marker;}

    /// Перед обработкой в СУБД
    bool isValid() const { return m_isValid; }

private:
    data_type_marker_record type_marker;
    bool m_isValid;
};

}

#endif // COMMAND_SERVER_MAP_CREATE_TYPE_MARKERS_H

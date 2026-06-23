#ifndef COMMAND_SERVER_MAP_UPDATE_TYPE_MARKERS_H
#define COMMAND_SERVER_MAP_UPDATE_TYPE_MARKERS_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"
#include "../../common/data/data_map_marker.h" // Путь к вашей структуре data_type_marker_record
#include <QDebug>

namespace server_protocol {

/// Обновить существующий тип меток (Отправка с клиента на сервер)
class command_server_map_type_markers_update : public protocol_message,
                                               public command {
public:

    /// ПРИЕМ НА СЕРВЕРЕ (Десериализация)
    explicit command_server_map_type_markers_update(const QByteArray& bodyData) :
        protocol_message(id_msg_command_server),
        command(id_command_server_map_update_type_markers),
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
            qWarning() << "command_server_map_type_markers_update: Data is empty!";
            return;
        }

        // Восстанавливаем обновленные данные типа метки из байт
        type_marker = data_type_marker_record(data, offset);

        // ЗАЩИТА: Проверяем, что структура считалась полностью
        // Тип не может быть обновлен, если у него пустая целевая цепочка или стерто имя
        if (type_marker.hierarchy_chain.isEmpty() || type_marker.name.isEmpty()) {
            qWarning() << "command_server_map_type_markers_update: Received corrupted or incomplete update record!";
            return;
        }

        m_isValid = true;
    }

    /// ОТПРАВКА С КЛИЕНТА (Сериализация)
    explicit command_server_map_type_markers_update(const data_type_marker_record& type_marker_) :
        protocol_message(id_msg_command_server),
        command(id_command_server_map_update_type_markers),
        type_marker(type_marker_),
        m_isValid(true)
    {
        // Оптимизация памяти: резервируем под 1 байт команды + размер упакованной структуры
        QByteArray serializedRecord = type_marker.toByteArray();
        data.reserve(1 + serializedRecord.size());

        // Код команды (1 байт)
        data.append(static_cast<char>(id_cmd));

        // Данные типа метки
        data.append(serializedRecord);
    }

    virtual ~command_server_map_type_markers_update() override = default;

    // Геттер для извлечения измененной записи типа
    data_type_marker_record get_type_marker() const { return type_marker; }

    // Проверка валидности пакета перед запуском SQL-транзакции изменения
    bool isValid() const { return m_isValid; }

private:
    data_type_marker_record type_marker;
    bool m_isValid;
};

} // namespace server_protocol

#endif // COMMAND_SERVER_MAP_UPDATE_TYPE_MARKERS_H

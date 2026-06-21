#ifndef COMMAND_SERVER_MAP_RESULT_REQUREQ_TYPE_MARKERS_H
#define COMMAND_SERVER_MAP_RESULT_REQUREQ_TYPE_MARKERS_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"
#include "../../common/data/data_map_marker.h"
#include <QDebug>

namespace server_protocol {

/// Ответ серверу от клиента: получил (или не смог получить) типы меток
// snapshot - нужен чтоб сервер учитывал по состоянию на какое время клиент знает об типах
class command_server_map_result_requreq_type_markers : public protocol_message,
                                                       public command {
public:

    /// ПРИЕМ НА СЕРВЕРЕ
    explicit command_server_map_result_requreq_type_markers(const QByteArray& bodyData) :
        protocol_message(id_msg_command_server),
        command(id_command_server_map_result_requreq_type_markers),
        snapshot(),
        result(invalid), // ИСПРАВЛЕНО: Безопасная инициализация на случай битого пакета
        m_isValid(false)
    {
        this->data = bodyData;
        int offset = 0;
        int totalSize = data.size();

        // Пропускаем id_cmd (1 байт)
        if (offset + 1 <= totalSize) {
            offset += 1;
        } else {
            qWarning() << "command_server_map_result_requreq_type_markers: Body data is empty!";
            return;
        }

        /// Результат (1 байт)
        if (offset + 1 <= totalSize) {
            result = static_cast<results_requreq>(data.at(offset)); // ИСПРАВЛЕНО: Использование .at()
            offset += 1;
        } else {
            qWarning() << "command_server_map_result_requreq_type_markers: Not enough bytes to read 'result' field!";
            return;
        }

        // Дата и время снимка данных
        const QString dtStr = readStringFromByteArray(data, offset);
        snapshot = QDateTime::fromString(dtStr, data_map_marker::format_lastUpdate);

        if (!snapshot.isValid()) {
            qWarning() << "command_server_map_result_requreq_type_markers: Received invalid snapshot QDateTime string:" << dtStr;
            return;
        }

        m_isValid = true;
    }

    /// ОТПРАВКА С КЛИЕНТА
    command_server_map_result_requreq_type_markers(const QDateTime& snapshot_, results_requreq result_) :
        protocol_message(id_msg_command_server),
        command(id_command_server_map_result_requreq_type_markers),
        snapshot(snapshot_),
        result(result_),
        m_isValid(true)
    {
        // Оптимизация: резервируем память (1 байт cmd + 1 байт result + ~25 байт дата)
        data.reserve(32);

        // Сначала добавляем идентификатор конкретной команды (1 байт)
        data.append(static_cast<char>(id_cmd));

        /// Результат (1 байт)
        data.append(static_cast<char>(result));

        // Добавляем дату и время снимка данных (строки запишутся с 2 байтами длины в Big-Endian)
        appendStringToByteArray(snapshot.toString(data_map_marker::format_lastUpdate), data);
    }

    // На случай удаления через интерфейсы
    virtual ~command_server_map_result_requreq_type_markers() override = default;

    // Геттеры
    QDateTime getSnapshot() const { return snapshot; }
    results_requreq getResult() const { return result; } // ИСПРАВЛЕНО: Избавились от принудительного каста (results_requreq)

    // Функция проверки: успешно ли распарсился пакет
    bool isValid() const { return m_isValid; }

private:
    QDateTime snapshot;
    results_requreq result;
    bool m_isValid;
};

} // namespace server_protocol

#endif // COMMAND_SERVER_MAP_RESULT_REQUREQ_TYPE_MARKERS_H

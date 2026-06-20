#ifndef COMMAND_CLIENT_MAP_UPDATE_TYPE_MARKER_H
#define COMMAND_CLIENT_MAP_UPDATE_TYPE_MARKER_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"

#include "../command_client.h"

namespace server_protocol {

class command_client_map_update_type_marker:    public protocol_message,
                                                public command {

public:

    command_client_map_update_type_marker(const QByteArray& bodyData):
        protocol_message(id_msg_command_client),
        command(id_command_client_map_update_type_marker),
        type_id(255),
        icon_format(unknown),
        offset_icon(-1)
    {
        // Сохраняем пришедшие байты в базовый класс
        this->data = bodyData;
        int offset = 0;
        int totalSize = data.size();

        // Пропускаем id_cmd (1 байт)
        if (offset + 1 <= totalSize) {
            offset += 1;
        }

        // Безопасно считываем 1 байт номера типа (type_id)
        if (offset + 1 <= totalSize) {
            type_id = static_cast<uint8_t>(data.at(offset));
            offset += 1;
        }

        /// Название типа
        name = readStringFromByteArray(data, offset);

        // Безопасно считываем 1 байт номера формата изображения (icon_format)
        if (offset + 1 <= totalSize) {
            icon_format = static_cast<format_icons>(data.at(offset));
            offset += 1;
        }

        /// Изображение
        // Запоминаем смещение только если оно в пределах массива
        if (offset <= totalSize) {
            offset_icon = offset;
        }
    }

    command_client_map_update_type_marker(uint8_t type_id_, const QString& name_, format_icons icon_format_, const QByteArray& icon_):
        protocol_message(id_msg_command_client),
        command(id_command_client_map_update_type_marker),
        type_id(type_id_), name(name_),
        icon_format(icon_format_)
    {
        // Оптимизация аллокации памяти
        data.reserve(1 + 1 + name.size() * 2 + 1 + icon_.size());

        // Сначала добавляем идентификатор конкретной команды (1 байт)
        data.append(static_cast<char>(id_cmd));

        // Номер типа
        data.append(static_cast<char>(type_id));

        /// Название типа
        appendStringToByteArray(name, data);

        // Какой формат изображения
        data.append(static_cast<char>(icon_format));

        /// Изображение
        // Фиксируем точку старта бинарных данных картинки
        offset_icon = data.size();
        data.append(icon_);
    }

    // На случай удаления через интерфейсы
    virtual ~command_client_map_update_type_marker() override = default;

    // Геттеры для доступа к данным из кода
    uint8_t getTypeId() const { return type_id; }
    QString getName() const { return name; }
    format_icons getIconFormat() const { return icon_format; }
    QByteArray getIcon() const {
        if (offset_icon < 0 || offset_icon > data.size()) {
            return QByteArray();
        }

        return data.mid(offset_icon);
    }

private:
    uint8_t type_id;
    QString name;
    format_icons icon_format;
    int offset_icon;
};

}

#endif // COMMAND_CLIENT_MAP_UPDATE_TYPE_MARKER_H

#ifndef PROTOCOL_MESSAGE_H
#define PROTOCOL_MESSAGE_H

#include <QByteArray>
#include <utility>

#include <QDataStream>
#include <QDebug>

namespace server_protocol {

// Типы сообщений
enum id_message: uint8_t{
    id_msg_command_server_map, // команда для сервера, в части карты
    id_msg_command_server_user,
    id_msg_command_client, // команда для клиента
    id_msg_text_info
};

// Базовый класс для всех сообщений
class protocol_message{
public:
    QByteArray data;
    uint8_t id_msg;

    //
    protocol_message() = default;

    // Узнать размер сообщения
    int size() const{
        return 1 + data.size();
    }

    // Создать сообщение по данным
    protocol_message(QByteArray&& data_, id_message id_msg_) noexcept:
       data(std::move(data_)), id_msg(id_msg_) {/* ... */}

    // Конструктор перемещения
    protocol_message(protocol_message&& other) noexcept:
       data(std::move(other.data)), id_msg(other.id_msg) {/* ... */}

    // Оператор перемещения
    protocol_message& operator=(protocol_message&&other) noexcept
    {
       if (this != &other)
       {
            id_msg = other.id_msg;
            data = std::move(other.data);
       }
       return *this;
    }
};

    // Для быстрой упаковки в QDataStream
    inline QDataStream& operator<<(QDataStream& out, const protocol_message& msg)
    {
        out << (uint8_t)msg.id_msg << msg.data;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, protocol_message& msg)
    {
        in >> msg.id_msg >> msg.data;
        return in;
    }
}

#endif // PROTOCOL_MESSAGE_H

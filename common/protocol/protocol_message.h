#ifndef PROTOCOL_MESSAGE_H
#define PROTOCOL_MESSAGE_H

#include <QByteArray>
#include <utility>

#include <QDataStream>
#include <QDebug>

namespace server_protocol {

// Типы сообщений в протоколе
enum id_message: uint8_t{
    id_msg_command_server, // команда для сервера
    id_msg_command_client, // команда для клиента
    id_msg_text_info
};

// Базовый класс для всех сообщений
class protocol_message{
public:
    //
    protocol_message() = default;

    // Узнать размер сообщения
    int size() const{
        return 1 + data.size();
    }

    /// Для отправки
    QByteArray toByteArray() const{
        QByteArray byteArray;
        byteArray.append(static_cast<char>(id_msg));
        byteArray.append(data);

        return byteArray;
    }

    // Создать сообщение по данным
    protocol_message(QByteArray&& data_, id_message id_msg_) noexcept:
       data(std::move(data_)), id_msg(id_msg_) {/* ... */}

private:
    // Из чего состоят все сообщения
    QByteArray data;
    uint8_t id_msg;
};

}

#endif // PROTOCOL_MESSAGE_H

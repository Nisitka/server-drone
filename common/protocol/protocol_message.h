#ifndef PROTOCOL_MESSAGE_H
#define PROTOCOL_MESSAGE_H

#include <QByteArray>
#include <QDataStream>
#include <QDebug>

namespace server_protocol {

// Типы сообщений в протоколе
enum id_message: uint8_t{
    id_msg_unknown,
    id_msg_command_server, // команда для сервера
    id_msg_command_client, // команда для клиента
    id_msg_text_info
};

// Базовый класс для всех сообщений
class protocol_message{
public:
    /// Для отправки
    virtual QByteArray toByteArray() const = 0;

    static uint8_t get_msg_id(const QByteArray& data_msg) {
        if (!data_msg.isEmpty()) {
            // id сообщения лежит в самом начале
            return static_cast<uint8_t>(data_msg[0]);
        }
        else {
            qDebug() << "get_msg_id: data_msg.isEmpty()!";
            return  id_msg_unknown;
        }
    }

    uint8_t get_msg_id() const {return id_msg;}

    protocol_message(id_message id_msg_):
        id_msg(id_msg_) {/* ... */}

protected:
    const uint8_t id_msg;
};

/// Вспомогательные функции
// Добавить в строку в массив данных
inline void appendStringToByteArray(const QString& str, QByteArray& box){
    const QByteArray strData = str.toUtf8();
    const int strLen = strData.size();

    // Добавляем длину строки
    box.append(reinterpret_cast<const char*>(&strLen), sizeof(strLen));

    // Добавляем сами данные строки
    box.append(strData);
}

inline QString readStringFromByteArray(const QByteArray& byteArray, int& pos_end, int offset = 0){
    const char* dataPtr = byteArray.constData();

    // Считаем длину строки
    int readStrLen;
    memcpy(&readStrLen, dataPtr + offset, sizeof(readStrLen));
    offset += sizeof(readStrLen);

    // Считаем строку
    const QByteArray readStrData(dataPtr + offset, readStrLen);

    pos_end = offset + readStrLen;
    return QString::fromUtf8(readStrData);
}

}

#endif // PROTOCOL_MESSAGE_H

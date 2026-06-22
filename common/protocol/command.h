#ifndef COMMAND_H
#define COMMAND_H

#include "./protocol_message.h"

#include <QDataStream>
#include <QIODevice>

namespace server_protocol {

class command{
public:

    command(uint8_t id_cmd_): id_cmd(id_cmd_){
        /* ... */
    }
    virtual ~command() = default;

    uint8_t id_command() const {return id_cmd;}

protected:
    const uint8_t id_cmd;

};

/// ---- Универсальный ответ для любой команды ----
class result_command: public protocol_message{

public:
    result_command(const QByteArray& bodyData):
        protocol_message(id_msg_result_command)
    {
        // Сохраняем пришедшие байты в базовый класс
        this->data = bodyData;

        // Сначала идет id команды результат которой мы хотим отправить
        int offset = 0;
        if (offset + sizeof(uint8_t) <= static_cast<size_t>(data.size())) {
            id_command = static_cast<uint8_t>(data[offset]);
            offset += sizeof(uint8_t);
        } else return;

        // Безопасно считываем 1 байт результата (result_code)
        if (offset + sizeof(results_requreq) <= static_cast<size_t>(data.size())) {
            result_code = static_cast<results_requreq>(data[offset]);
            offset += sizeof(uint8_t);
        } else return;

        /// Доп. информация об результате выполнения команды
        if (offset + sizeof(uint8_t) <= static_cast<size_t>(data.size())) {
            id_info = static_cast<uint8_t>(data[offset]);
        } else return;
    }

    result_command(uint8_t id_command_, results_requreq result_code_, uint8_t info_id = 0):
        protocol_message(id_msg_result_command),
        id_command(id_command_), result_code(result_code_), id_info(info_id)
    {
        data.append(static_cast<char>(id_command));
        data.append(static_cast<char>(result_code));
        data.append(static_cast<char>(id_info));
    }

    uint8_t id_cmd() const {return id_command;}
    results_requreq code_result() const {return result_code;}
    uint8_t info() const {return id_info;}

private:
    uint8_t id_command;
    results_requreq result_code;
    uint8_t id_info; /// каждая команда содержит свой список значений доп. информации
};

}

#endif // COMMAND_H

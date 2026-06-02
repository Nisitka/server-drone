#ifndef COMMANDS_CLIENT_USER_RESULT_AUTH_H
#define COMMANDS_CLIENT_USER_RESULT_AUTH_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"

#include "../command_client.h"

namespace server_protocol {

class command_client_user_result_auth : public protocol_message,
                                        public command {
public:
    enum results_auth : uint8_t {
        successfully,
        invalid_login_or_password,
        invalid
    };

    // -------------------------------------------------------------
    // Сценарий 1: ПРИЕМ НА СТОРОНЕ КЛИЕНТА (Конструктор десериализации)
    // -------------------------------------------------------------
    // Сюда передается чистый bodyData (уже без 4 байт сетевого заголовка протокола)
    command_client_user_result_auth(const QByteArray& bodyData) :
        protocol_message(id_msg_command_client),
        command(id_command_client_user_result_auth),
        value(invalid) // Дефолтное значение на случай повреждения пакета
    {
        // Сохраняем пришедшие байты в базовый класс
        this->data = bodyData;

        int offset = 0;

        // Пропускаем id_cmd (1 байт)
        if (offset + sizeof(uint8_t) <= static_cast<size_t>(data.size())) {
            offset += sizeof(uint8_t);
        }

        // Безопасно считываем 1 байт результата (value) с проверкой границ
        if (offset + sizeof(uint8_t) <= static_cast<size_t>(data.size())) {
            value = static_cast<results_auth>(data[offset]);
        } else {
            qWarning() << "command_client_user_result_auth: Недостаточно байт для чтения поля value!";
        }
    }

    // -------------------------------------------------------------
    // Сценарий 2: ОТПРАВКА С СЕРВЕРА (Конструктор сериализации)
    // -------------------------------------------------------------
    command_client_user_result_auth(results_auth value_) :
        protocol_message(id_msg_command_client),
        command(id_command_client_user_result_auth),
        value(value_)
    {
        // Формируем внутреннее тело (data). Внешний заголовок базовый класс добавит сам!

        // Сначала добавляем идентификатор конкретной команды (1 байт)
        data.append(static_cast<char>(id_cmd));

        // Добавляем 1 байт результата авторизации
        data.append(static_cast<char>(value));
    }

    // Возвращаем строгое перечисление вместо uint8_t для удобства в switch/case
    results_auth Value() const {
        return value;
    }

private:
    results_auth value;
};

}
#endif // COMMANDS_CLIENT_USER_RESULT_AUTH_H

#ifndef COMMAND_SERVER_USER_AUTH_H
#define COMMAND_SERVER_USER_AUTH_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"

namespace server_protocol {

// Авторизоваться на сервере: логин (QString), пароль (QString)
class command_server_user_auth : public protocol_message, public command {
public:
    // -------------------------------------------------------------
    // Сценарий 1: ПРИЕМ ИЗ СЕТИ (Конструктор десериализации)
    // -------------------------------------------------------------
    // Сюда должен передаваться ЧИСТЫЙ bodyData (уже без заголовка протокола 4 байт)
    command_server_user_auth(const QByteArray& bodyData) :
        protocol_message(id_msg_command_server),
        command(id_command_server_user_auth)
    {
        // Нам нужно сохранить пришедшие байты в базовый класс
        this->data = bodyData;

        int offset = 0;

        // Сначала считываем id_cmd (1 байт), так как он идет первым в теле команды
        if (offset + sizeof(uint8_t) <= static_cast<size_t>(data.size())) {
            // Если id_cmd нужно проверить или сохранить, считываем его.
            // Мы знаем, что он равен id_command_server_user_auth, поэтому просто сдвигаем offset
            offset += sizeof(uint8_t);
        }

        // Поочередно извлекаем строки
        login = readStringFromByteArray(data, offset);
        pass  = readStringFromByteArray(data, offset);
    }

    // -------------------------------------------------------------
    // Сценарий 2: СОЗДАНИЕ ДЛЯ ОТПРАВКИ (Конструктор сериализации)
    // -------------------------------------------------------------
    command_server_user_auth(const QString& login_, const QString& pass_) :
        protocol_message(id_msg_command_server),
        command(id_command_server_user_auth),
        login(login_), pass(pass_)
    {
        // Формируем ВНУТРЕННЕЕ тело (data) для этой конкретной команды.
        // Внешний заголовок протокола (MagicByte, id_msg, общий размер) базовый класс добавит сам

        // Сначала добавляем идентификатор конкретной команды (1 байт)
        data.append(static_cast<char>(id_cmd));

        // Добавляем прикладные данные (строки запишутся с 2 байтами длины в Big-Endian)
        appendStringToByteArray(login, data);
        appendStringToByteArray(pass, data);
    }

    const QString& Login() const { return login; }
    const QString& Password() const { return pass; }

private:
    QString login;
    QString pass;
};

}

#endif // COMMAND_SERVER_USER_AUTH_H

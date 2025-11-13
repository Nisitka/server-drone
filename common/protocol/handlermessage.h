#ifndef HANDLERMESSAGE_H
#define HANDLERMESSAGE_H

#include <QByteArray>

#include "./protocol_message.h"

// Класс-интерфейс для всех обработчиков сообщений
class HandlerMessage
{
public:

    // Какой тип сообщений обрабатывает экземпляр
    const server_protocol::id_message id_proc_msg;

    HandlerMessage(server_protocol::id_message id_proc_msg_):
        id_proc_msg(id_proc_msg_)
    {/* ... */}

    // Обработать сообщение
    virtual bool processingMessage(QByteArray& data) = 0;
};

#endif // HANDLERMESSAGE_H

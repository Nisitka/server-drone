#ifndef HANDLERMESSAGECOMMANDSERVERMAP_H
#define HANDLERMESSAGECOMMANDSERVERMAP_H

#include "../../../common/protocol/handlermessage.h"

// Обработчик сообщений-команд для карты сервера
class HandlerMessageCommandServerMap: public HandlerMessage
{
public:
    HandlerMessageCommandServerMap();

    bool processingMessage(QByteArray &data) override final;
};

#endif // HANDLERMESSAGECOMMANDSERVERMAP_H

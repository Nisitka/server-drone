#ifndef ISOCKETADAPTER_H
#define ISOCKETADAPTER_H

#include <QObject>
#include <QByteArray>
#include "../protocol/protocol_message.h" // Подключаем, чтобы интерфейс знал про id_message

// Непотокобезопасная обертка для QTcpSocket
// при испускании сигнала message() - ожидает обработку в том же потоке
class ISocketAdapter: public QObject {
    Q_OBJECT

signals:
    void message();
    void disconnected();

    void trSendByteArray(const QByteArray& data);

public:
    explicit ISocketAdapter();
    virtual ~ISocketAdapter() = default;

    // Тело последнего сообщения
    QByteArray getCurrentMessage() const {
        return currentMessage;
    }

    // Тип последнего сообщения верхнего уровня
    server_protocol::id_message getLastMsgId() const {
        return lastMsgId;
    }

protected slots:
    // Отправка данных
    virtual void sendByteArray(const QByteArray& data) = 0;

protected:
    // Последнее полученное сообщение (чистое тело)
    QByteArray currentMessage;

    // Хранилище состояния парсинга текущего пакета
    server_protocol::MessageHeader currentHeader;

    // Идентификатор для типа последнего успешно принятого сообщения
    server_protocol::id_message lastMsgId;
};

#endif // ISOCKETADAPTER_H

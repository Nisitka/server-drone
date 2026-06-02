#ifndef SOCKETADAPTER_H
# define SOCKETADAPTER_H

#include "../protocol/protocol_message.h"

# include "isocketadapter.h"

#include <QTcpSocket>
#include <QByteArray>
#include "../protocol/protocol_message.h"
#include "isocketadapter.h"

class SocketAdapter: public ISocketAdapter {
    Q_OBJECT

public:
    explicit SocketAdapter(QTcpSocket *pSock = nullptr);
    virtual ~SocketAdapter();

    // Геттеры для внешних фабрик (вызываются в слоте, подключенном к сигналу message())
    server_protocol::id_message getLastMsgId() const { return lastMsgId; }
    const QByteArray& getCurrentMessage() const { return currentMessage; }

public slots:
    // Отключить соединение
    void disconnect();

protected slots:
    void readyRead();

    // Переопределение метода отправки данных
    void sendByteArray(const QByteArray& data) override final;

protected:
    QTcpSocket* tcpSocket;

};

#endif // SOCKETADAPTER_H

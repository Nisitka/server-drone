#ifndef ISOCKETADAPTER_H
#define ISOCKETADAPTER_H

#include <QObject>
#include <QByteArray>

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
    virtual ~ISocketAdapter();

    // Забрать данные последнего сообщения
    QByteArray getCurrentMessage()const{
        return currentMessage;
    }

protected slots:
    virtual void sendByteArray(const QByteArray& data) = 0;

protected:
    // Последнее полученное сообщение
    QByteArray currentMessage;
};

#endif // ISOCKETADAPTER_H

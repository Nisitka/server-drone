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

public:
    explicit ISocketAdapter(QObject *parent);
    virtual ~ISocketAdapter();

    virtual void sendByteArray(const QByteArray& data) = 0;

    // Забрать данные последнего сообщения
    const QByteArray& getCurrentMessage()const{
        return currentMessage;
    }

protected:
    // Последнее полученное сообщение
    QByteArray currentMessage;
};

#endif // ISOCKETADAPTER_H

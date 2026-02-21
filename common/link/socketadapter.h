#ifndef SOCKETADAPTER_H
# define SOCKETADAPTER_H

# include "isocketadapter.h"

class QTcpSocket;
class SocketAdapter: public ISocketAdapter {
  Q_OBJECT

public slots:
    // Отключить соединение
    void disconnect();

public:
    SocketAdapter(QTcpSocket *pSock = nullptr);
    virtual ~SocketAdapter();

protected slots:
    void readyRead();
    void sendByteArray(const QByteArray& data) override final;

protected:
    QTcpSocket* tcpSocket;
    qint16 msgSize;
};

#endif // SOCKETADAPTER_H

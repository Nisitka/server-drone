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
  SocketAdapter(QObject *parent, QTcpSocket *pSock = nullptr);
  virtual ~SocketAdapter();

  virtual void sendByteArray(const QByteArray& data);

protected slots:
  void readyRead();

protected:
  QTcpSocket* tcpSocket;
  qint16 msgSize;
};

#endif // SOCKETADAPTER_H

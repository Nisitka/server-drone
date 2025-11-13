#ifndef SERVERSOCKETADAPTER_H
# define SERVERSOCKETADAPTER_H

# include "../../common/link/socketadapter.h"

class ServerSocketAdapter : public SocketAdapter {
  Q_OBJECT
public:
  explicit ServerSocketAdapter(QTcpSocket* pSock, QObject *parent = nullptr);
};

#endif // SERVERSOCKETADAPTER_H

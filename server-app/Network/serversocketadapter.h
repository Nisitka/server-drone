#ifndef SERVERSOCKETADAPTER_H
# define SERVERSOCKETADAPTER_H

# include "../../common/link/socketadapter.h"

class ServerSocketAdapter : public SocketAdapter {
    Q_OBJECT
public:
  explicit ServerSocketAdapter(QTcpSocket* pSock);
};

#endif // SERVERSOCKETADAPTER_H

#include "serversocketadapter.h"
#include <QTcpSocket>

ServerSocketAdapter::ServerSocketAdapter(QTcpSocket* pSock):
    SocketAdapter(pSock)
{

}

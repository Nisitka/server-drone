#ifndef SERVERSOCKETADAPTER_H
# define SERVERSOCKETADAPTER_H

# include "../../common/link/socketadapter.h"

#include <QTimer>

class ServerSocketAdapter: public SocketAdapter {
    Q_OBJECT
public:
    explicit ServerSocketAdapter(QTcpSocket* pSock);

private slots:
    // Отключить сокет из-за бездействия
    void shutdownDueInactivity();

private:
    // Таймер, который определяет бездействие со стороны клиента
    QTimer* inactivityTimer;
};

#endif // SERVERSOCKETADAPTER_H

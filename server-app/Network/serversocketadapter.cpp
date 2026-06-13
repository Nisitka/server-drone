#include "serversocketadapter.h"
#include <QTcpSocket>

#include <QDebug>

#include "../../common/protocol/heartbeat.h"

ServerSocketAdapter::ServerSocketAdapter(QTcpSocket* pSock):
    SocketAdapter(pSock)
{
    /// ----- Отключение клиентов при долгой тишине с его стороны -----------------------
    inactivityTimer = new QTimer(this); // Удаляем таймер при удалении сокета
    inactivityTimer->setInterval(server_protocol::HEARTBEAT_INTERVAL * 3);
    inactivityTimer->setSingleShot(true); // чтоб срабатывал только один раз

    connect(inactivityTimer, &QTimer::timeout,
            this,            &ServerSocketAdapter::shutdownDueInactivity);

    connect(this, &SocketAdapter::message, inactivityTimer, [this]() {
        inactivityTimer->start();
    });
    inactivityTimer->start();
}

void ServerSocketAdapter::shutdownDueInactivity()
{
    qDebug() << "ServerSocketAdapter: shutdown due inactivity";

    /// Жесткое отключение (альтернатива - disconnectFromHost())
    this->disconnect();
}

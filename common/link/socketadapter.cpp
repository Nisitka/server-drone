#include "socketadapter.h"

#include <QVector>
#include <QTcpSocket>
#include <QDataStream>

SocketAdapter::SocketAdapter(QObject *parent, QTcpSocket* tcpSocket_):
    ISocketAdapter(parent),
    tcpSocket(tcpSocket_),
    msgSize(-1)
{
    // Если адаптируем еще не созданный socket
    if (!tcpSocket)
        tcpSocket = new QTcpSocket(this);

    /// Чтение пакета данных - подготовка сообщения
    connect(tcpSocket, &QTcpSocket::readyRead,
            this,      &SocketAdapter::readyRead);

    // Уведомляем об отключении соединения
    connect(tcpSocket, &QTcpSocket::disconnected,
            this,      &SocketAdapter::disconnected);

    currentMessage.clear();
}

void SocketAdapter::readyRead() {
    QDataStream stream(tcpSocket);

    // Пока не считаем всю полезную информацию их текущих данных в буфере сокета
    while(true) {

        // Если началось новое сообщение (block)
        if (msgSize < 0) // То пытаемся считать его размер
        {
            /// Для теста
            if (!currentMessage.isEmpty())
                qDebug() << "SocketAdapter::" << "Получен новый блок данных, хотя предыдущее сообщение никто не забрал!!!";

            // Если длинны хватает для извлечения размера сообщения
            if (tcpSocket->bytesAvailable() >= sizeof(msgSize))
            {
                // Извлекаем размер ожидаемого сообщения
                stream >> msgSize;
                qDebug() << "SocketAdapter: start accepted msg, size -" << msgSize;

                // Подготавливаем блок для нового сообщения
                currentMessage.clear();
                currentMessage.resize(msgSize);
            }
            else
                // ждем следующий блок данных
                // (данных недостаточно даже чтоб считать размер грядущего сообщения)
                return;
        }
        else
        {
            qDebug() << "check tcpSocket->bytesAvailable() >= msgSize";

            // Если длины хватает для извлечения данных сообщения
            if (tcpSocket->bytesAvailable() >= msgSize)
            {
                // Извлекаем данные сообщения
                stream.readRawData(currentMessage.data(), msgSize);
                msgSize = -1; // флаг нового сообщения

                // Уведомляем о том, что можно забрать сообщение
                emit message(); /// в этом же потоке сразу начинается обработка этого сообщения
            }
            else
                // ждем следующий блок
                //(данных размер сообщения есть, но пришло не все)
                return;
        }
    }
}

void SocketAdapter::sendByteArray(const QByteArray& data) {
  QByteArray block;
  QDataStream sendStream(&block, QIODevice::ReadWrite);

  // sendStream << quint16(0) << data;

  // sendStream.device()->seek(0);
  // sendStream << (quint16)(block.size() - sizeof(quint16));

  sendStream << (quint16)data.size() << data;

  qDebug() << "SocketAdapter: send msg, size -" << (quint16)block.size();
  tcpSocket->write(block);
}

void SocketAdapter::disconnect()
{
    if (tcpSocket->isOpen())
    {
        tcpSocket->close();
        emit disconnected();
    }
}

SocketAdapter::~SocketAdapter()
{
    disconnect();

    tcpSocket->deleteLater();
}

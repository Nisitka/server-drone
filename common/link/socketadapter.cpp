#include "socketadapter.h"

#include "socketadapter.h"
#include <QDebug>
#include <QtEndian>

namespace server_protocol {
// Если методы parseHeader еще не были вынесены в cpp,
// компилятор найдет их через заголовочный файл protocol_message.h
}

/**
 * @brief Конструктор адаптера сокета
 * @param pSock — указатель на существующий QTcpSocket (если nullptr, создается новый)
 */
SocketAdapter::SocketAdapter(QTcpSocket* pSock) :
    ISocketAdapter(),
    tcpSocket(pSock)
{
    // Если сокет не был передан извне (например, при создании клиента), инициализируем новый
    if (!tcpSocket) {
        tcpSocket = new QTcpSocket(this);
    } else {
        // Если сокет передан (например, сервером при входящем подключении),
        // делаем адаптер его родителем для автоматического управления памятью
        tcpSocket->setParent(this);
    }

    // Подключаем асинхронный сигнал readyRead для пошагового накопления байт
    connect(tcpSocket, &QTcpSocket::readyRead,
            this,      &SocketAdapter::readyRead);

    // Подключаем сигнал отключения для уведомления бизнес-логики верхнего уровня
    connect(tcpSocket, &QTcpSocket::disconnected,
            this,      &SocketAdapter::disconnected);

    // Сбрасываем структуру заголовка и внутренний буфер сообщения
    currentHeader = server_protocol::MessageHeader();
    currentMessage.clear();
}

/**
 * @brief Деструктор адаптера сокета
 */
SocketAdapter::~SocketAdapter() {
    // Метод deleteLater безопасен, так как предотвращает удаление сокета
    // во время выполнения его собственных внутренних слотов обработки событий
    tcpSocket->deleteLater();
}

/**
 * @brief Асинхронный слот обработки входящего потока байт (Защищен от фрагментации TCP)
 */
void SocketAdapter::readyRead() {
    // Цикл while необходим на случай, если в одном событии readyRead
    // в буфер сокета прилетело сразу несколько пакетов один за другим.
    while (true) {

        // ШАГ 1: Если заголовок текущего пакета еще не прочитан (или сброшен)
        if (!currentHeader.isValid) {

            // Минимальный размер сетевого заголовка в нашем протоколе — 4 байта
            if (tcpSocket->bytesAvailable() >= 4) {

                // Подглядываем (peek) первые 4 байта. Они ОСТАЮТСЯ в буфере ОС,
                // что защищает данные от потери, если пакет пришел не целиком.
                QByteArray headerData = tcpSocket->peek(4);

                // Передаем байты в наш статический кроссплатформенный парсер
                currentHeader = server_protocol::protocol_message::parseHeader(headerData);

                // Если MagicByte не совпал — поток рассинхронизирован (атака или сбой связи)
                if (!currentHeader.isValid) {
                    qWarning() << "SocketAdapter: Критическая ошибка! Неверная сигнатура MagicByte. Соединение разорвано.";
                    disconnect();
                    return;
                }

                qDebug() << "SocketAdapter: Заголовок успешно распознан. ID:"
                         << currentHeader.msgId << "| Ожидаемый размер тела:" << currentHeader.bodySize << "байт";
            }
            else {
                // Данных в сокете меньше 4 байт, прерываем цикл и ждем следующую порцию в readyRead
                return;
            }
        }

        // ШАГ 2: Если заголовок успешно распознан, проверяем наличие всего тела пакета
        if (currentHeader.isValid) {

            // Общий размер полного пакета (Заголовок 4 байта + Полезная нагрузка bodySize)
            qint64 totalPacketSize = 4 + currentHeader.bodySize;

            // Если весь пакет целиком гарантированно загружен в буфер сокета
            if (tcpSocket->bytesAvailable() >= totalPacketSize) {

                // Извлекаем (read) весь пакет, физически удаляя его из буфера сокета
                QByteArray fullPacket = tcpSocket->read(totalPacketSize);

                // Отсекаем первые 4 байта сетевого заголовка, оставляя только чистые прикладные данные
                currentMessage = fullPacket.mid(4);

                // Сохраняем ID сообщения, чтобы внешняя фабрика знала, как его интерпретировать
                lastMsgId = currentHeader.msgId;

                // Сбрасываем состояние заголовка в невалидное, чтобы на следующем витке
                // цикла while начать разбор нового пакета, если он есть в буфере
                currentHeader = server_protocol::MessageHeader();

                // Генерируем сигнал message(). Подключенный к нему менеджер
                // сразу же заберет данные через геттерыgetLastMsgId() и getCurrentMessage()
                emit message();
            }
            else {
                // Заголовок прочитан, но тело пакета еще долетает по сети.
                // Выходим из цикла, состояние currentHeader сохраняется до следующего readyRead.
                return;
            }
        }
    }
}

/**
 * @brief Безопасная отправка сообщения в сеть
 * @param data — сформированный массив байт, возвращенный методом .toByteArray() класса команды
 */
void SocketAdapter::sendByteArray(const QByteArray& data) {
    if (!tcpSocket) {
        qWarning() << "SocketAdapter::sendByteArray: Указатель на сокет равен nullptr!";
        return;
    }

    if (!tcpSocket->isOpen() || !tcpSocket->isWritable()) {
        qWarning() << "SocketAdapter::sendByteArray: Сокет закрыт или недоступен для записи!";
        return;
    }

    // Вся сборка заголовка (Magic, ID, Size) уже выполнена на уровне базового класса команды.
    // Адаптер просто отправляет этот монолитный массив байт напрямую в сеть.
    qint64 bytesWritten = tcpSocket->write(data);

    if (bytesWritten == -1) {
        qCritical() << "SocketAdapter::sendByteArray: Ошибка при записи данных в сокет:" << tcpSocket->errorString();
    } else {
        qDebug() << "SocketAdapter: Успешно отправлен пакет размером" << bytesWritten << "байт.";
    }
}

/**
 * @brief Принудительное закрытие соединения
 */
void SocketAdapter::disconnect() {
    if (tcpSocket && tcpSocket->isOpen()) {
        qDebug() << "SocketAdapter: Инициировано закрытие соединения сокета.";
        tcpSocket->close();
    }
}



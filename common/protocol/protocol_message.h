#ifndef PROTOCOL_MESSAGE_H
#define PROTOCOL_MESSAGE_H

#include <QByteArray>
#include <QDataStream>
#include <QtEndian>

#include <QDebug>

namespace server_protocol {

// Значение - идентификатор протокола (Magic Byte)
inline const uint8_t MagicByte = 0xEF; // 239 в dec (или 0xFE = 254)

// Типы сообщений в протоколе
enum id_message : uint8_t {
    id_msg_unknown,
    id_msg_command_server, // команда для сервера
    id_msg_command_client, // команда для клиента
    id_msg_text_info
};

// Структура для хранения информации о распарсенном заголовке
struct MessageHeader {
    bool isValid = false;
    id_message msgId = id_msg_unknown;
    uint16_t bodySize = 0;
};

// Базовый класс для всех сообщений
class protocol_message {
public:
    // Виртуальный деструктор ОБЯЗАТЕЛЕН для базовых классов
    virtual ~protocol_message() = default;

    protocol_message(id_message id_msg_) : id_msg(id_msg_) {}

    uint8_t get_msg_id() const { return id_msg; }

    /// Сборка пакета для отправки
    QByteArray toByteArray() const {
        QByteArray byteArray;

        // 1 байт (Magic) + 1 байт (ID) + 2 байта (Size) = 4 байта заголовок
        byteArray.reserve(4 + data.size());

        byteArray.append(static_cast<char>(MagicByte));
        byteArray.append(static_cast<char>(id_msg));

        if (data.size() > std::numeric_limits<uint16_t>::max()) {
            qCritical() << "Размер данных" << data.size() << "превышает лимит uint16_t!";
            return QByteArray();
        }

        uint16_t size = static_cast<uint16_t>(data.size());
        uint16_t dataSize = qToBigEndian(size);

        byteArray.append(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
        byteArray.append(data);

        return byteArray;
    }

    /**
     * @brief Безопасный парсинг заголовка (статический метод)
     * Используется ПЕРЕД тем, как читать тело сообщения из сокета.
     */
    static MessageHeader parseHeader(const QByteArray& headerBuffer) {
        MessageHeader header;

        // Заголовок должен быть строго не меньше 4 байт
        if (headerBuffer.size() < 4) {
            return header;
        }

        // Проверяем MagicByte (что это сообщение нашего протокола)
        if (static_cast<uint8_t>(headerBuffer[0]) != MagicByte) {
            qDebug() << "parseHeader: Неверный MagicByte!";
            return header;
        }

        // Извлекаем ID сообщения
        header.msgId = static_cast<id_message>(headerBuffer[1]);

        // Извлекаем и конвертируем размер тела (из Big-Endian)
        uint16_t rawSize;
        // Безопасно копируем 2 байта размера из буфера
        std::memcpy(&rawSize, headerBuffer.constData() + 2, sizeof(uint16_t));
        header.bodySize = qFromBigEndian(rawSize);

        header.isValid = true;
        return header;
    }

protected:
    // id сообщения
    const uint8_t id_msg;

    /// Тело данных
    QByteArray data; // Заполняется в дочерних классах перед отправкой
};

/// Вспомогательные функции
// Добавить строку в массив данных (Безопасная кроссплатформенная версия)
inline void appendStringToByteArray(const QString& str, QByteArray& box) {
    const QByteArray strData = str.toUtf8();

    // Проверяем, не превышает ли строка лимиты (на случай аномально больших строк)
    if (strData.size() > std::numeric_limits<uint16_t>::max()) {
        qCritical() << "Размер строки превышает лимит uint16_t!";
        return;
    }

    // Стандартизируем тип данных — строго 2 байта для длины строки
    uint16_t strLen = static_cast<uint16_t>(strData.size());

    // Конвертируем в сетевой порядок байт (Big-Endian)
    uint16_t networkLen = qToBigEndian(strLen);

    // Добавляем 2 байта длины строки
    box.append(reinterpret_cast<const char*>(&networkLen), sizeof(networkLen));

    // Добавляем само тело строки
    box.append(strData);
}

inline QString readStringFromByteArray(const QByteArray& box, int& offset) {
    // Проверяем, хватает ли данных хотя бы на чтение длины (2 байта)
    if (offset + 2 > box.size()) return QString();

    // Читаем 2 байта длины строки
    uint16_t rawLen;
    std::memcpy(&rawLen, box.constData() + offset, sizeof(uint16_t));
    offset += 2;

    // Переводим из сетевого порядка байт обратно
    uint16_t strLen = qFromBigEndian(rawLen);

    // Проверяем, хватает ли оставшихся байт в массиве на всю строку
    if (offset + strLen > box.size()) return QString();

    // Извлекаем байты строки и двигаем смещение (offset) вперед
    QByteArray strData = box.mid(offset, strLen);
    offset += strLen;

    return QString::fromUtf8(strData);
}

}

#endif // PROTOCOL_MESSAGE_H

#ifndef PROTOCOL_MESSAGE_H
#define PROTOCOL_MESSAGE_H

#include <QByteArray>
#include <QDataStream>
#include <QtEndian>

#include <QDebug>

namespace server_protocol {

// Значение - идентификатор протокола (Magic Byte)
inline const uint8_t MagicByte = 0xEF; // 239 в dec (или 0xFE = 254)

// Инициализирующее значение CRC по стандарту MAVLink X.25
inline const uint16_t INIT_CRC_VALUE = 0xFFFF;

// Типы сообщений в протоколе
enum id_message: uint8_t {
    id_msg_unknown        = 0,
    id_msg_command_server = 1, // команда для сервера
    id_msg_command_client = 2, // команда для клиента
    id_msg_text_info      = 3
};

/**
 * @brief Побайтовое накопление контрольной суммы MAVLink (X.25 CRC-16)
 */
inline void crc_accumulate(uint8_t data, uint16_t *crcAccum) {
    // мы применяем операцию XOR (Исключающее ИЛИ) между новым байтом данных data и этим младшим байтом CRC. Результат временно сохраняется в 8-битную переменную tmp
    uint8_t tmp = data ^ (uint8_t)(*crcAccum & 0xFF); // мы берем текущую 16-битную контрольную сумму и «отрезаем» от нее только младший (правый) байт.
    tmp ^= (tmp << 4); // Полубайтовое «размножение» (Вычисление промежуточного полинома)
    *crcAccum = (*crcAccum >> 8) ^ (tmp << 8) ^ (tmp << 3) ^ (tmp >> 4);
}

// Структура для хранения информации о распарсенном заголовке
struct MessageHeader {
    bool isValid = false;
    id_message msgId = id_msg_unknown;
    uint16_t bodySize = 0;
    int totalPacketSize = 0; // Это позволит избавиться от ручного подсчета "4 + bodySize + 2" в сокете.
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

        // Резервируем: 4 байта заголовок + тело + 2 байта CRC
        byteArray.reserve(4 + data.size() + 2);

        // Стартовый маркер
        byteArray.append(static_cast<char>(MagicByte));

        // Буфер для полей, покрываемых CRC (ID, размер тела, само тело)
        QByteArray crcCoveredBuffer;
        crcCoveredBuffer.reserve(1 + 2 + data.size());
        crcCoveredBuffer.append(static_cast<char>(id_msg));

        if (data.size() > std::numeric_limits<uint16_t>::max()) {
            qCritical() << "Размер данных превышает лимит uint16_t!";
            return QByteArray();
        }

        // Сначала добавляем размер тела данных в байтах
        uint16_t size = static_cast<uint16_t>(data.size());
        uint16_t dataSize = qToBigEndian(size);
        crcCoveredBuffer.append(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
        // Затем сами данные
        crcCoveredBuffer.append(data);

        // Считаем контрольную сумму (исключая MagicByte)
        uint16_t crc = INIT_CRC_VALUE;
        for (int i = 0; i < crcCoveredBuffer.size(); ++i) {
            crc_accumulate(static_cast<uint8_t>(crcCoveredBuffer[i]), &crc);
        }

        // Собираем финальный пакет
        byteArray.append(crcCoveredBuffer);

        // Записываем 2 байта CRC в конец (используем Little-Endian для CRC)
        uint16_t leCrc = qToLittleEndian(crc);
        byteArray.append(reinterpret_cast<const char*>(&leCrc), sizeof(leCrc));

        return byteArray;
    }

    /**
     * @brief Безопасный парсинг заголовка (статический метод)
     * Используется ПЕРЕД тем, как читать тело сообщения из сокета.
     */
    static MessageHeader parseHeader(const QByteArray& headerBuffer) {
        MessageHeader header;

        // если данных недостаточно для целого заголовка
        if (headerBuffer.size() < 4) return header;

        if (static_cast<uint8_t>(headerBuffer[0]) != MagicByte) {
            qDebug() << "parseHeader: Неверный MagicByte!";
            return header;
        }

        // id сообщения
        header.msgId = static_cast<id_message>(headerBuffer[1]);

        uint16_t rawSize;
        std::memcpy(&rawSize, headerBuffer.constData() + 2, sizeof(uint16_t));
        header.bodySize = qFromBigEndian(rawSize);

        // ВЫЧИСЛЯЕМ И ФИКСИРУЕМ ПОЛНЫЙ РАЗМЕР ПАКЕТА С УЧЕТОМ 2 БАЙТ CRC
        // 4 байта заголовка + размер тела + 2 байта CRC в хвосте
        header.totalPacketSize = 4 + header.bodySize + 2;

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

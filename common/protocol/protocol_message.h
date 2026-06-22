#ifndef PROTOCOL_MESSAGE_H
#define PROTOCOL_MESSAGE_H

#include <QByteArray>
#include <QDataStream>
#include <QtEndian>

#include <QDebug>

namespace server_protocol {

/// Значение - идентификатор протокола
inline const uint8_t MagicByte = 0xEF; // 239 в десятичной

// Инициализирующее значение CRC по стандарту X.25
inline const uint16_t INIT_CRC_VALUE = 0xFFFF;

// Максимальный разумный размер тела пакета для защиты от DOS-атак (например, 5 МБ)
inline const uint32_t MAX_ALLOWED_BODY_SIZE = 5 * 1024 * 1024;

// Типы изображений (форматы)
enum format_icons: uint8_t {
    svg = 0,
    png = 1,
    jpg = 2,

    unknown = 255
};

/// Универсальные коды возврата
enum results_requreq: uint8_t {
    successfully = 0,
    error        = 1,

    invalid      = 29
};

// Типы сообщений в протоколе
enum id_message: uint8_t {
    id_msg_unknown        = 255,

    id_msg_heartbeat      = 0, /// Сердцебиение

    id_msg_command_server = 1, // команда для сервера
    id_msg_command_client = 2, // команда для клиента
    id_msg_result_command = 3, /// результат выполнения любой команды

    id_msg_text_info      = 10
};

/**
 * @brief Побайтовое накопление контрольной суммы (X.25 CRC-16)
 */
inline void crc_accumulate(uint8_t data, uint16_t *crcAccum) {
    // мы применяем операцию XOR (исключающее ИЛИ) между новым байтом данных data и этим младшим байтом CRC. Результат временно сохраняется в 8-битную переменную tmp
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

    protocol_message(id_message id_msg_): id_msg(id_msg_) {
        data.clear();
    }

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

        if (header.bodySize > MAX_ALLOWED_BODY_SIZE) {
            qWarning() << "parseHeader: Превышен максимальный лимит размера тела пакета!";
            header.isValid = false;
            return header;
        }

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
    QByteArray strData = str.toUtf8();
    uint16_t strLen = 0;

    // Проверяем, не превышает ли строка лимиты (на случай аномально больших строк)
    if (strData.size() > std::numeric_limits<uint16_t>::max()) {
        qCritical() << "Размер строки превышает лимит uint16_t!";
        strData = strData.left(std::numeric_limits<uint16_t>::max());
    }

    // Стандартизируем тип данных — строго 2 байта для длины строки
    strLen = static_cast<uint16_t>(strData.size());

    // Конвертируем в сетевой порядок байт (Big-Endian)
    uint16_t networkLen = qToBigEndian(strLen);

    // Добавляем 2 байта длины строки
    box.append(reinterpret_cast<const char*>(&networkLen), sizeof(networkLen));

    // Добавляем само тело строки
    box.append(strData);
}

inline QString readStringFromByteArray(const QByteArray& box, int& offset) {
    if (offset + 2 > box.size()) return QString();

    uint16_t rawLen;
    std::memcpy(&rawLen, box.constData() + offset, sizeof(uint16_t));
    offset += 2;

    uint16_t strLen = qFromBigEndian(rawLen);

    if (offset + strLen > box.size()) return QString();

    // Безопасно: QString СРАЗУ копирует данные к себе в UTF-16.
    // box.constData() + offset — это просто прямой указатель на память.
    QString result = QString::fromUtf8(box.constData() + offset, strLen);

    // Только после успешного создания строки сдвигаем offset
    offset += strLen;

    return result;
}

}

#endif // PROTOCOL_MESSAGE_H

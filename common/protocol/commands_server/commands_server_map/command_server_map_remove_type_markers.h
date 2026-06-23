#ifndef COMMAND_SERVER_MAP_REMOVE_TYPE_MARKERS_H
#define COMMAND_SERVER_MAP_REMOVE_TYPE_MARKERS_H

#include "../../../protocol/command.h"
#include "../../../protocol/protocol_message.h"
#include "../../commands_server/command_server.h"

#include <QDebug>
#include <QtEndian>
#include <cstring>

namespace server_protocol {

/// Запрос от клиента на удаление типа меток (по его цепочке иерархии)
class command_server_map_remove_type_markers : public protocol_message,
                                               public command {
public:

    /// ПРИЕМ НА СЕРВЕРЕ (Десериализация)
    explicit command_server_map_remove_type_markers(const QByteArray& bodyData) :
        protocol_message(id_msg_command_server),
        command(id_command_server_map_remove_type_markers),
        m_isValid(false)
    {
        this->data = bodyData;
        int offset = 0;
        int totalSize = data.size();

        //  Пропускаем id_cmd (1 байт)
        if (offset + 1 <= totalSize) {
            offset += 1;
        } else {
            qWarning() << "command_server_map_remove_type_markers: Data is empty!";
            return;
        }

        // Читаем размер цепочки иерархии (2 байта, Big-Endian)
        if (offset + 2 > totalSize) {
            qWarning() << "command_server_map_remove_type_markers: Not enough bytes to read chain size!";
            return;
        }
        uint16_t rawChainSize;
        std::memcpy(&rawChainSize, data.constData() + offset, sizeof(uint16_t));
        offset += 2;
        uint16_t chainSize = qFromBigEndian(rawChainSize);

        // Читаем саму цепочку ID (по 1 байту на каждый ID)
        if (offset + chainSize > totalSize) {
            qWarning() << "command_server_map_remove_type_markers: Packet is truncated, cannot read full chain!";
            return;
        }

        hierarchy_chain.clear();
        hierarchy_chain.reserve(chainSize);
        for (uint16_t i = 0; i < chainSize; ++i) {
            hierarchy_chain.append(static_cast<uint8_t>(data.at(offset)));
            offset += 1;
        }

        // Защита: цепочка на удаление не должна быть пустой
        if (hierarchy_chain.isEmpty()) {
            qWarning() << "command_server_map_remove_type_markers: Received an empty hierarchy chain for deletion!";
            return;
        }

        m_isValid = true;
    }

    /// ОТПРАВКА С КЛИЕНТА (Сериализация)
    explicit command_server_map_remove_type_markers(const QList<uint8_t>& chain_to_remove) :
        protocol_message(id_msg_command_server),
        command(id_command_server_map_remove_type_markers),
        hierarchy_chain(chain_to_remove),
        m_isValid(true)
    {
        // Резервируем память под 1 байт cmd + 2 байта длины + элементы списка
        data.reserve(1 + 2 + hierarchy_chain.size());

        // Записываем код команды (1 байт)
        data.append(static_cast<char>(id_cmd));

        // Записываем размер цепочки иерархии (2 байта, Big-Endian)
        uint16_t chainSize = static_cast<uint16_t>(hierarchy_chain.size());
        uint16_t networkChainSize = qToBigEndian(chainSize);
        data.append(reinterpret_cast<const char*>(&networkChainSize), sizeof(uint16_t));

        // Записываем каждый ID цепочки (по 1 байту)
        for (uint8_t id : hierarchy_chain) {
            data.append(static_cast<char>(id));
        }
    }

    virtual ~command_server_map_remove_type_markers() override = default;

    // Геттер для получения целевой цепочки удаления
    QList<uint8_t> getHierarchyChain() const { return hierarchy_chain; }

    // Проверка валидности пакета перед отправкой в пул потоков базы данных
    bool isValid() const { return m_isValid; }

private:
    // Цепочка типа, который нужно удалить
    QList<uint8_t> hierarchy_chain;

    bool m_isValid;
};

} // namespace server_protocol

#endif // COMMAND_SERVER_MAP_REMOVE_TYPE_MARKERS_H

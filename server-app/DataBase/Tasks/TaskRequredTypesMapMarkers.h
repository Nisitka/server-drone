#ifndef TASKREQUREDTYPESMAPMARKERS_H
#define TASKREQUREDTYPESMAPMARKERS_H

#include "./taskdatabase.h"
#include "../../Network/ActionsClientsManager.h"

#include "../../../common/protocol/commands_server/commands_server_map/command_server_map_requreq_type_markers.h"
#include "../../../common/protocol/commands_client/commands_client_map/command_client_map_result_requreq_type_markers.h"

#include <QDateTime>
#include <QSqlError>

using namespace server_protocol;

class TaskRequredTypesMapMarkers: public TaskDataBase
{
public:
    TaskRequredTypesMapMarkers(ActionsClientsManager* clientsManager_,
                               const QString& uuid_client_,
                               const command_server_map_requreq_type_markers& cmd_requreq):
        TaskDataBase(""), // Первоначально передаем пустую строку, так как базовый класс требует её в конструктор
        clientsManager(clientsManager_),
        uuid_client(uuid_client_),
        isForced(cmd_requreq.isForcedUpdate())
    {
        /// В зависимости от параметра у нас разный SQL-запрос
        if (isForced) {
            stringSQL = "SELECT * FROM get_all_marker_types();";
        } else {// Защищаем строковый параметр uuid_client с помощью $$
            stringSQL = QString("SELECT * FROM get_marker_types_delta($$%1$$);").arg(uuid_client);
        }
    }

    bool processRequestResult(QSqlQuery& query) override final
    {
        /// ЗАЩИТА ОТ СБОЕВ СУБД:
        // Проверяем, произошла ли критическая ошибка при выполнении SQL-запроса
        if (query.lastError().isValid()) {
            qWarning() << "TaskRequredTypesMapMarkers: critical error SQL-request!"
                       << query.lastError().text();

            // Отправляем клиенту статус invalid, так как запрос физически сломался
            command_client_map_result_requreq_type_markers cmd(
                invalid,
                QDateTime::currentDateTime(),
                QList<command_client_map_result_requreq_type_markers::TypeRecord>{},
                QList<QList<uint8_t>>{}
                );
            emit clientsManager->sendByteArray(uuid_client, cmd.toByteArray());

            return false; // Сигнализируем пулу потоков, что задача завершилась со сбоем
        }

        if (isForced) {
            return proccesingRequestResult_forcedRequred(query);
        }
        else {
            // Новые/обновленные типы
            QList<command_client_map_result_requreq_type_markers::TypeRecord> types_list;
            // Типы, которые удалили
            QList<QList<uint8_t>> deleted_chains_list;

            /// Считываем данные для типов
            while (query.next())
            {
                // Получаем цепочку типов текущей строки
                const QString hierarchy_chain_str = query.value(2).toString();
                QList<uint8_t> hierarchy_chain = data_map_marker::parseHierarchyString(hierarchy_chain_str);

                if (hierarchy_chain.isEmpty()) {
                    qWarning() << "TaskRequredTypesMapMarkers: An empty or broken hierarchy row is obtained:" << hierarchy_chain_str;
                    continue;
                }

                // Проверяем флаг удаления
                const bool isDeleted = query.value(3).toBool();
                if (isDeleted) {
                    deleted_chains_list.append(hierarchy_chain);
                    continue; // Переходим к следующей записи, картинку не качаем
                }

                // Читаем метаданные живого типа
                const QString name_type = query.value(0).toString();
                const QByteArray iconBytes = query.value(1).toByteArray();

                command_client_map_result_requreq_type_markers::TypeRecord record;
                record.hierarchy_chain = hierarchy_chain;
                record.name = name_type;
                record.iconBytes = iconBytes;

                types_list.append(record);
            }

            // Компануем команду клиенту
            QDateTime date_time = QDateTime::currentDateTime();

            // Если дельта пустая (изменений нет) — отправляем статус successfully и пустые списки
            results_requreq result = successfully;

            command_client_map_result_requreq_type_markers cmd(result, date_time, types_list, deleted_chains_list);
            emit clientsManager->sendByteArray(uuid_client, cmd.toByteArray());

            return true;
        }
    }

private:
    bool proccesingRequestResult_forcedRequred(QSqlQuery& query) const
    {
        // В случае полного дерева удаления игнорируются
        QList<command_client_map_result_requreq_type_markers::TypeRecord> types_list;

        while (query.next())
        {
            const QString hierarchy_chain_str = query.value(2).toString();
            QList<uint8_t> hierarchy_chain = data_map_marker::parseHierarchyString(hierarchy_chain_str);

            if (hierarchy_chain.isEmpty()) continue;

            const QString name_type = query.value(0).toString();
            const QByteArray iconBytes = query.value(1).toByteArray();

            command_client_map_result_requreq_type_markers::TypeRecord record;
            record.hierarchy_chain = hierarchy_chain;
            record.name = name_type;
            record.iconBytes = iconBytes;

            types_list.append(record);
        }

        QDateTime date_time = QDateTime::currentDateTime();
        results_requreq result = successfully;

        // Если база пустая и типов вообще нет (например, первый старт сервера) — ставим ошибку
        if (types_list.isEmpty()) {
            qWarning() << "TaskRequredTypesMapMarkers (Forced): database did not return a single type!";
            result = error;
        }

        command_client_map_result_requreq_type_markers cmd(result, date_time, types_list, QList<QList<uint8_t>>{});
        emit clientsManager->sendByteArray(uuid_client, cmd.toByteArray());

        return true;
    }

    ActionsClientsManager* clientsManager;
    const QString uuid_client;
    bool isForced;
};

#endif // TASKREQUREDTYPESMAPMARKERS_H

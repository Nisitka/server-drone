#ifndef TASKREQUREDTYPESMAPMARKERS_H
#define TASKREQUREDTYPESMAPMARKERS_H

#include "./taskdatabase.h"
#include "../../Network/ActionsClientsManager.h"

#include "../../../common/protocol/commands_server/commands_server_map/command_server_map_requreq_type_markers.h"
#include "../../../common/protocol/commands_client/commands_client_map/command_client_map_result_requreq_type_markers.h"

#include <QDateTime>

using namespace server_protocol;

class TaskRequredTypesMapMarkers: public TaskDataBase
{
public:
    TaskRequredTypesMapMarkers(ActionsClientsManager* clientsManager_,
                               const QString& uuid_client_,
                               const command_server_map_requreq_type_markers& cmd_requreq):
    TaskDataBase(""),
    uuid_client(uuid_client_),
    clientsManager(clientsManager_),
    isForced(cmd_requreq.isForcedUpdate())
    {
        if (isForced)
            stringSQL = "SELECT * FROM get_all_marker_types()";
        else
            stringSQL = "SELECT * FROM get_marker_types_delta('" + uuid_client + "')";
    }

    bool processRequestResult(QSqlQuery& query) override final
    {
        if (isForced){

            return proccesingRequestResult_forcedRequred(query);
        }
        else{

            // Новые типы
            QList<command_client_map_result_requreq_type_markers::TypeRecord> types_list;

            // Типы которые удалили
            QList<QList<uint8_t>> deleted_chains_list;

            // id типа (цепочка типов)
            const QString hierarchy_chain_str = query.value(2).toString();
            QList <uint8_t> hierarchy_chain = data_map_marker::parseHierarchyString(hierarchy_chain_str);

            while (query.next())
            {
                // Удален ли этот тип
                const bool isDeleted = query.value(3).toBool();

                if (isDeleted){
                    deleted_chains_list.append(hierarchy_chain);
                    continue;
                }

                // Имя типа
                const QString name_type = query.value(0).toString();

                // Изображение типа
                const QByteArray iconBytes = query.value(1).toByteArray();

                command_client_map_result_requreq_type_markers::TypeRecord record;
                record.hierarchy_chain = hierarchy_chain;
                record.name = name_type;
                record.iconBytes = iconBytes;

                types_list.append(record);
            }

            /// Компануем команду клиенту
            QDateTime date_time = QDateTime::currentDateTime();
            results_requreq result = successfully;

            command_client_map_result_requreq_type_markers cmd(result, date_time, types_list, deleted_chains_list);
            emit clientsManager->sendByteArray(uuid_client, cmd.toByteArray());

            return true;
        }
    }

private:
    bool proccesingRequestResult_forcedRequred(QSqlQuery& query) const{

        // В случае если получаем все типы с самого нуля, нам неважно какие типы были удалены
        QList<command_client_map_result_requreq_type_markers::TypeRecord> types_list;

        while (query.next())
        {
            // Имя типа
            const QString name_type = query.value(0).toString();

            // Изображение типа
            const QByteArray iconBytes = query.value(1).toByteArray();

            // id типа в виде строки
            const QString hierarchy_chain = query.value(2).toString();

            command_client_map_result_requreq_type_markers::TypeRecord record;
            record.hierarchy_chain = data_map_marker::parseHierarchyString(hierarchy_chain);
            record.name = name_type;
            record.iconBytes = iconBytes;

            types_list.append(record);
        }

        /// Компануем команду клиенту
        QDateTime date_time = QDateTime::currentDateTime();
        results_requreq result = invalid;
        // Если ни одного типа нет, значит какая-то херня
        if (!types_list.isEmpty()){
            result = successfully;
        }
        else{
            result = invalid;
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

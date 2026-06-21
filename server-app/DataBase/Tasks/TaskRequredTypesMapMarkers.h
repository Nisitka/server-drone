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
        QList<command_client_map_result_requreq_type_markers::TypeRecord> typesList;

        while (query.next())
        {
            const QString name_type = query.value(0).toString();

            // id типа в виде строки
            const QString hierarchy_chain = query.value(1).toString();

            const QByteArray iconBytes = query.value(2).toByteArray();

            command_client_map_result_requreq_type_markers::TypeRecord record;
            record.hierarchy_chain = data_map_marker::parseHierarchyString(hierarchy_chain);
            record.name = name_type;
            record.iconBytes = iconBytes;

            typesList.append(record);
        }

        /// Компануем команду клиенту
        QDateTime date_time = QDateTime::currentDateTime();
        results_requreq result = invalid;
        if (!typesList.isEmpty()){
            result = successfully;
        }
        else{
            result = invalid;
        }

        command_client_map_result_requreq_type_markers cmd(result, date_time, typesList);
        emit clientsManager->sendByteArray(uuid_client, cmd.toByteArray());

        return true;
    }

private:
    ActionsClientsManager* clientsManager;
    const QString uuid_client;

    bool isForced;
};

#endif // TASKREQUREDTYPESMAPMARKERS_H

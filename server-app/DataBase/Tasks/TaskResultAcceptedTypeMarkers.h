#ifndef TASKRESULTACCEPTEDTYPEMARKERS_H
#define TASKRESULTACCEPTEDTYPEMARKERS_H

#include "../../../common/protocol/commands_server/commands_server_map/command_server_map_result_requreq_type_markers.h"

#include "./taskdatabase.h"

#include <QDateTime>

using namespace server_protocol;

class TaskResultAcceptTypeMarkers: public TaskDataBase
{
public:
    TaskResultAcceptTypeMarkers(const QString& uuid_client_,
                                const command_server_map_result_requreq_type_markers& cmd_accepted):
        TaskDataBase("SELECT * FROM mark_marker_types_synced('" + uuid_client_ + "', '"
                                                                + cmd_accepted.getSnapshot().toString(data_map_marker::format_lastUpdate) + "')"),
        uuid_client(uuid_client_)
    {

    }

    bool processRequestResult(QSqlQuery& query) override final
    {
        if (query.next()){
            int code_result = query.value(0).toInt();
            qDebug() << "TaskResultAcceptTypeMarkers: code_result -" << code_result << ", user -" << uuid_client;
        }
        else
            return false;

        return true;
    }

private:
    const QString uuid_client;

};

#endif // TASKRESULTACCEPTEDTYPEMARKERS_H

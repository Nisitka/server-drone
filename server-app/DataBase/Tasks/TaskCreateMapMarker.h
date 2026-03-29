#ifndef TASKCREATEMAPMARKER_H
#define TASKCREATEMAPMARKER_H

#include "./taskdatabase.h"
#include "../../Network/ActionsClientsManager.h"

#include "../../../common/protocol/commands_server/commands_server_map/command_server_map_object_create.h"
#include "../../../common/protocol/commands_client/commands_client_map/command_client_map_object_created.h"

using namespace server_protocol;

class TaskCreateMapMarker: public TaskDataBase
{
public:
    TaskCreateMapMarker(ActionsClientsManager* clientsManager_,
                        const QString& login_client,
                        const command_server_map_object_create& m):
    TaskDataBase("SELECT * FROM __CreateMarkerById("
        + QString::number(m.getDataMarker().lat) + ","
        + QString::number(m.getDataMarker().lon) + ","

        + "'" + m.getDataMarker().name + "',"
        + "'" + m.getDataMarker().info + "',"

        + "ARRAY["  + QString::number(m.getDataMarker().colorName.red()) + ","
        + QString::number(m.getDataMarker().colorName.green()) + ","
        + QString::number(m.getDataMarker().colorName.blue()) + "],"

        + "'" + QString::number(m.getDataMarker().type_obj_id) + "-"
        + QString::number(m.getDataMarker().subtype_obj_id) + "',"

        + "'" + m.getDataMarker().uuid + "',"

        + "'" + m.getDataMarker().lastUpdate.toString(data_map_marker::format_lastUpdate) + "'"

        + ");"),
        data_marker(m.getDataMarker()),
        login(login_client),
        clientsManager(clientsManager_)
    {/* ... */}

    bool processRequestResult(QSqlQuery& query) override final{

        command_client_map_object_created cmd_obj_created(data_marker);

        // Уведомляем другие клиенты об этом
        emit clientsManager->sendByteArrayAllUsersExcept(QStringList{login},
                                                         cmd_obj_created.toByteArray());

        return true;
    }
private:
    ActionsClientsManager* clientsManager;
    const QString login;
    const data_map_marker data_marker;
};

#endif // TASKCREATEMAPMARKER_H

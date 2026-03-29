#ifndef TASKUPDATEMAPMARKER_H
#define TASKUPDATEMAPMARKER_H

#include "./taskdatabase.h"
#include "../../Network/ActionsClientsManager.h"

#include "../../../common/protocol/commands_server/commands_server_map/command_server_map_object_update.h"
#include "../../../common/protocol/commands_client/commands_client_map/command_client_map_object_update.h"

using namespace server_protocol;

class TaskUpdateMapMarker: public TaskDataBase
{
public:
    TaskUpdateMapMarker(ActionsClientsManager* clientsManager_,
                        const QString& login_client,
                        const command_server_map_object_update& cmd):
    login(login_client),
    clientsManager(clientsManager_),
    data_marker(cmd.getDataMarker()),
    TaskDataBase("SELECT * FROM __ChangeInfoMarker('" +
        cmd.getDataMarker().uuid + "',"

        + QString::number(cmd.getDataMarker().lat) + ","
        + QString::number(cmd.getDataMarker().lon) + ","

        + "'" + cmd.getDataMarker().lastUpdate.toString(data_map_marker::format_lastUpdate) + "',"

        + "'" + cmd.getDataMarker().name + "',"
        + "'" + cmd.getDataMarker().info + "',"

        + "ARRAY["  + QString::number(cmd.getDataMarker().colorName.red()) + ","
        + QString::number(cmd.getDataMarker().colorName.green()) + ","
        + QString::number(cmd.getDataMarker().colorName.blue()) + "],"

        + "'" + QString::number(cmd.getDataMarker().type_obj_id) + "-"
        + QString::number(cmd.getDataMarker().subtype_obj_id) + "'"

        + ");")
    {}

    bool processRequestResult(QSqlQuery& query) override final{
        if (!query.next()) return false;
        else
        {
            int code = query.value(0).toInt();
            switch (code) {
            case 0:{
                qDebug() << "TaskUpdateMapMarker: code 0";
                command_client_map_object_update cmd_update_marker(data_marker);

                /// Уведомляем других об изменениях
                // кроме клиента, который сообщил об изменениях
                emit clientsManager->sendByteArrayAllUsersExcept(QStringList{login},
                                                                 cmd_update_marker.toByteArray());
                break;}

            case 1:
                qDebug() << "TaskUpdateMapMarker: code 1";
                /// Сообщаем клиенту что не удалось обновить данные метки

                break;

            default:
                qDebug() << "TaskUpdateMapMarker: unknown return code query...";
                break;
            }
        }

        return true;
    }

private:
    ActionsClientsManager* clientsManager;
    const data_map_marker data_marker;
    const QString login;
};

#endif // TASKUPDATEMAPMARKER_H

#ifndef TASKREMOVEMAPMARKER_H
#define TASKREMOVEMAPMARKER_H

#include "./taskdatabase.h"
#include "../../Network/ActionsClientsManager.h"

#include "../../../common/protocol/commands_server/commands_server_map/command_server_map_remove_object.h"

#include "../../../common/protocol/commands_client/commands_client_map/command_client_map_object_removed.h"

using namespace server_protocol;

class TaskRemoveMapMarker: public TaskDataBase
{
public:
    TaskRemoveMapMarker(ActionsClientsManager* clientsManager_,
                        const QString& login_client,
                        const command_server_map_remove_object& cmd):
        TaskDataBase("SELECT * FROM __DeleteObject('"+ cmd.uuid_marker +"');"),
        clientsManager(clientsManager_),
        login(login_client),
        uuid_marker(cmd.uuid_marker)
    {}

    bool processRequestResult(QSqlQuery& query) override final{
        /// Уведомляюся все клиенты о том, что метка удалена
        command_client_map_object_removed cmd_obj_removed(uuid_marker);

        // За исключением того клиента, который сообщил об удалении
        emit clientsManager->sendByteArrayAllUsersExcept(QStringList{login},
                                                         cmd_obj_removed.toByteArray());

        return true;
    }

private:
    ActionsClientsManager* clientsManager;
    const QString login;
    const QString uuid_marker;
};

#endif // TASKREMOVEMAPMARKER_H

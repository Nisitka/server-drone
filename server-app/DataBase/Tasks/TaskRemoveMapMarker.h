#ifndef TASKREMOVEMAPMARKER_H
#define TASKREMOVEMAPMARKER_H

#include "./taskdatabase.h"
#include "../../Network/ActionsClientsManager.h"

#include "../../../common/protocol/commands_server/commands_server_map/command_server_map_remove_object.h"

#include "../../../common/protocol/commands_client/commands_client_map/command_client_map_object_removed.h"

using namespace server_protocol;

#include "./taskdatabase.h"
#include "../../Network/ActionsClientsManager.h"

#include "../../../common/protocol/commands_server/commands_server_map/command_server_map_remove_object.h"
#include "../../../common/protocol/commands_client/commands_client_map/command_client_map_object_removed.h"

#include <QSqlError>

class TaskRemoveMapMarker: public TaskDataBase
{
public:
    TaskRemoveMapMarker(ActionsClientsManager* clientsManager_,
                        const QString& uuid_client_,
                        const command_server_map_remove_object& cmd) :
        // Защищаем базу данных от SQL-инъекций, экранируя кавычки в пришедшем из сети UUID
        TaskDataBase(QString("SELECT * FROM __DeleteObject($$%1$$);").arg(cmd.uuid_marker)),
        clientsManager(clientsManager_),
        uuid_client(uuid_client_),
        uuid_marker(cmd.uuid_marker)
    {}

    bool processRequestResult(QSqlQuery& query) override final {

        /// ЗАЩИТА ОТ СБОЕВ СУБД:
        // Проверяем, произошла ли критическая ошибка при выполнении SQL-запроса
        results_requreq res_code = invalid;
        if (query.lastError().isValid()) {
            qWarning() << "TaskRemoveMapMarker: critical error SQL-request!"
                       << query.lastError().text();
        }
        // Проверяем поняла ли СУБД наш SQL-запрос
        else if (query.next()){
            int code = query.value(0).toInt();

            if (code == 0)
                res_code = successfully;
            else
                res_code = error;
        }

        /// Сообщаем автору команды результат её выполнения
        result_command msg_res_command(id_command_server_map_object_remove,
                                       res_code);
        emit clientsManager->sendByteArray(uuid_client, msg_res_command.toByteArray());

        /// Если SQL-запрос выполнен успешно то сообщаем другим клиентам об удалении метки
        if (res_code == successfully){

            /// Уведомляются все клиенты о том, что метка удалена
            // За исключением того клиента, который сообщил об удалении (инициатора)
            command_client_map_object_removed cmd_obj_removed(uuid_marker);
            emit clientsManager->sendByteArrayAllUsersExcept(QStringList{uuid_client},
                                                             cmd_obj_removed.toByteArray());

            qDebug() << "TaskRemoveMapMarker: object" << uuid_marker << "successfully deleted. The newsletter has been sent.";
        }

        // Возвращаем true, если пакет от СУБД в принципе был получен и обработан
        return res_code != invalid;
    }

private:
    ActionsClientsManager* clientsManager;
    const QString uuid_client;
    const QString uuid_marker;
};

#endif // TASKREMOVEMAPMARKER_H

#ifndef TASKRESULTACCEPTEDTYPEMARKERS_H
#define TASKRESULTACCEPTEDTYPEMARKERS_H

#include "./taskdatabase.h"
#include "../../Network/ActionsClientsManager.h"
#include "../../../common/protocol/commands_server/commands_server_map/command_server_map_result_requreq_type_markers.h"
#include <QSqlError>
#include <QDebug>

using namespace server_protocol;

class TaskResultAcceptTypeMarkers: public TaskDataBase
{
public:
    TaskResultAcceptTypeMarkers(ActionsClientsManager* clientsManager_,
                                const QString& uuid_client_,
                                const command_server_map_result_requreq_type_markers& cmd_accepted) :
        TaskDataBase(QString("SELECT * FROM mark_marker_types_synced($$%1$$, $$%2$$);")
                         .arg(uuid_client_)
                         .arg(cmd_accepted.getSnapshot().toString(data_map_marker::format_lastUpdate))),
        uuid_client(uuid_client_),
        clientsManager(clientsManager_)
    { }

    bool processRequestResult(QSqlQuery& query) override final
    {
        results_requreq res_code = invalid;

        // Если запрос физически сломался на уровне PostgreSQL
        if (query.lastError().isValid()) {
            qWarning() << "TaskResultAcceptTypeMarkers: Critical DBMS error for the user:" << uuid_client
                       << "| error:" << query.lastError().text();

            // Сообщаем клиенту, что пакет не обработан из-за сбоя сервера
            result_command msg_res_command(id_command_server_map_result_requreq_type_markers, invalid);
            emit clientsManager->sendByteArray(uuid_client, msg_res_command.toByteArray());
            return false;
        }

        if (query.next()) {
            int code_result = query.value(0).toInt();
            qDebug() << "TaskResultAcceptTypeMarkers: code_result -" << code_result << ", user -" << uuid_client;

            // Если функция в БД вернула 0, значит транзакция прошла успешно
            if (code_result == 0) {
                res_code = successfully;
            } else {
                res_code = error;
            }
        }
        else {
            qWarning() << "TaskResultAcceptTypeMarkers: The DBMS returned an empty result for the user:" << uuid_client;
            res_code = error;
        }

        // Сообщаем клиенту (автору подтверждения) результат обработки команды на сервере
        result_command msg_res_command(id_command_server_map_result_requreq_type_markers, res_code);
        emit clientsManager->sendByteArray(uuid_client, msg_res_command.toByteArray());

        return res_code != invalid;
    }

private:
    const QString uuid_client;
    ActionsClientsManager* clientsManager;
};

#endif // TASKRESULTACCEPTEDTYPEMARKERS_H

#ifndef TASKREMOVETYPEMARKER_H
#define TASKREMOVETYPEMARKER_H

#pragma once

#include "./taskdatabase.h"
#include "../../Network/ActionsClientsManager.h"
#include "../../../common/protocol/commands_server/commands_server_map/command_server_map_remove_type_markers.h"
#include "../../../common/protocol/commands_client/commands_client_map/command_client_map_result_requreq_type_markers.h"
#include "../../../common/protocol/common/data/data_map_marker.h"

#include <QSqlError>
#include <QDebug>

using namespace server_protocol;

class TaskRemoveTypeMarker : public TaskDataBase
{
public:
    TaskRemoveTypeMarker(ActionsClientsManager* clientsManager_,
                         const QString& uuid_client_,
                         const command_server_map_remove_type_markers& cmd):
        TaskDataBase(buildQuery(cmd)),
        clientsManager(clientsManager_),
        uuid_client(uuid_client_),
        hierarchy_chain(cmd.getHierarchyChain())
    { }

    bool processRequestResult(QSqlQuery& query) override final
    {
        // ЗАЩИТА: Проверяем системные ошибки базы данных
        if (query.lastError().isValid()) {
            qWarning() << "TaskRemoveTypeMarker: critical error from postgres!"
                       << query.lastError().text();

            result_command msg_res_command(id_command_server_map_remove_type_markers, invalid);
            emit clientsManager->sendByteArray(uuid_client, msg_res_command.toByteArray());
            return false;
        }

        // Разбираем ответ от хранимой процедуры
        results_requreq res_code = invalid;
        if (query.next()) {
            int code = query.value(0).toInt();

            if (code == 0) {
                res_code = successfully;
            } else {
                res_code = error;
            }
        } else {
            qWarning() << "TaskRemoveTypeMarker: postgres return empty result!";
            res_code = error;
        }

        // Отправляем автору команды результат её выполнения
        result_command msg_res_command(id_command_server_map_remove_type_markers, res_code);
        emit clientsManager->sendByteArray(uuid_client, msg_res_command.toByteArray());

        if (res_code == successfully) {
            qDebug() << "TaskRemoveTypeMarker: Marker's type successfully removed:"
                     << hierarchy_chain;

            // Тип, который только что создали
            QList<QList<uint8_t>>deleted_type;
            deleted_type.append(hierarchy_chain);

            // На какой момент времени будет отправлено состояние типов
            QDateTime date_time = QDateTime::currentDateTime();
            results_requreq result = successfully;

            // По какой причине был запрос типов
            command_client_map_result_requreq_type_markers::motive m_motive = command_client_map_result_requreq_type_markers::update;

            command_client_map_result_requreq_type_markers cmd(result, m_motive, date_time, QList<data_type_marker_record>{}, deleted_type);
            emit clientsManager->sendByteArrayAllUsersExcept(QStringList{}, cmd.toByteArray());
        }

        return res_code != invalid;
    }

private:
    // Вспомогательный статический метод для чистой сборки SQL-запроса
    static QString buildQuery(const command_server_map_remove_type_markers& cmd) {

        // Конвертируем QList<uint8_t> в строку иерархии через дефисы
        QString hierarchyStr = data_map_marker::makeHierarchyString(cmd.getHierarchyChain());

        return QString("SELECT * FROM __RemoveMarkerType($$%1$$);").arg(hierarchyStr);
    }

    ActionsClientsManager* clientsManager;
    const QString uuid_client;
    const QList<uint8_t> hierarchy_chain;
};

#endif // TASKREMOVETYPEMARKER_H

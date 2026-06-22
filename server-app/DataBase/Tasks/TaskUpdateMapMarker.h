#ifndef TASKUPDATEMAPMARKER_H
#define TASKUPDATEMAPMARKER_H

#include "./taskdatabase.h"
#include "../../Network/ActionsClientsManager.h"

#include "../../../common/protocol/commands_server/commands_server_map/command_server_map_object_update.h"
#include "../../../common/protocol/commands_client/commands_client_map/command_client_map_object_update.h"
#include <QLocale>
#include <QSqlError>
#include <QDebug>

using namespace server_protocol;

class TaskUpdateMapMarker: public TaskDataBase
{
public:
    TaskUpdateMapMarker(ActionsClientsManager* clientsManager_,
                        const QString& uuid_client_,
                        const command_server_map_object_update& cmd) :
        TaskDataBase(buildSecureQuery(cmd)),
        uuid_client(uuid_client_),
        clientsManager(clientsManager_),
        data_marker(cmd.getDataMarker())
    {}

    bool processRequestResult(QSqlQuery& query) override final {
        /// ОБРАБОТКА ОШИБОК СУБД: Если запрос физически сломался на уровне PostgreSQL
        if (query.lastError().isValid()) {
            qWarning() << "TaskUpdateMapMarker: Critical DBMS error!"
                       << query.lastError().text();

            // Отправляем статус invalid (сбой сервера/пакета)
            result_command msg_res_command(id_command_server_map_object_update, invalid);
            emit clientsManager->sendByteArray(uuid_client, msg_res_command.toByteArray());
            return false;
        }

        if (!query.next()) {
            qWarning() << "TaskUpdateMapMarker: DBMS return empty result for marker:" << data_marker.get_uuid();
            result_command msg_res_command(id_command_server_map_object_update, error);
            emit clientsManager->sendByteArray(uuid_client, msg_res_command.toByteArray());
            return false;
        }

        int code = query.value(0).toInt();
        switch (code) {
        case 0: {
            qDebug() << "TaskUpdateMapMarker: Successfully updated in the database. We're sending it to the others.";
            result_command msg_res_command(id_command_server_map_object_update, successfully);
            emit clientsManager->sendByteArray(uuid_client, msg_res_command.toByteArray());

            // Уведомляем других об изменениях, кроме инициатора
            command_client_map_object_update cmd_update_marker(data_marker);
            emit clientsManager->sendByteArrayAllUsersExcept(QStringList{uuid_client},
                                                             cmd_update_marker.toByteArray());
            break;
            }
        case 1: {
            qDebug() << "TaskUpdateMapMarker: Error changing tag data in the database (code 1)";
            result_command msg_res_command(id_command_server_map_object_update,
                                           error,
                                           command_server_map_object_update::invalid_date_or_time_changed);

            emit clientsManager->sendByteArray(uuid_client, msg_res_command.toByteArray());
            break;
            }
        default: {
            qDebug() << "TaskUpdateMapMarker: Unknown response code received from the database:" << code;
            result_command msg_res_command(id_command_server_map_object_update, error);
            emit clientsManager->sendByteArray(uuid_client, msg_res_command.toByteArray());
            break;
            }
        }

        return true;
    }

private:
    // Вспомогательный статический метод для чистой и безопасной сборки SQL строки
    static QString buildSecureQuery(const command_server_map_object_update& cmd) {
        const data_map_marker& marker = cmd.getDataMarker();

        // Используем C-локаль, чтобы double ВСЕГДА форматировался с точкой
        QLocale cLocale(QLocale::C);
        QString lonStr = cLocale.toString(marker.lon, 'f', 7);
        QString latStr = cLocale.toString(marker.lat, 'f', 7);

        QString hierarchyStr = marker.getHierarchyChain_str();
        QString timeStr = marker.lastUpdate.toString(data_map_marker::format_lastUpdate());

        // Применяем долларовые кавычки $$ для защиты текстовых полей.
        // Использование QString::arg делает код прозрачным и легко читаемым.
        return QString("SELECT * FROM __ChangeInfoMarker("
                       "'%1', %2, %3, '%4', $$%5$$, $$%6$$, ARRAY[%7, %8, %9], '%10'"
                       ");")
            .arg(marker.get_uuid())
            .arg(lonStr)
            .arg(latStr)
            .arg(timeStr)
            .arg(marker.name)
            .arg(marker.info)
            .arg(marker.colorName.red())
            .arg(marker.colorName.green())
            .arg(marker.colorName.blue())
            .arg(hierarchyStr);
    }

    ActionsClientsManager* clientsManager;
    const data_map_marker data_marker;
    const QString uuid_client;
};

#endif // TASKUPDATEMAPMARKER_H

#ifndef TASKCREATEMAPMARKER_H
#define TASKCREATEMAPMARKER_H

#include "./taskdatabase.h"
#include "../../Network/ActionsClientsManager.h"

#include "../../../common/protocol/commands_server/commands_server_map/command_server_map_object_create.h"
#include "../../../common/protocol/commands_client/commands_client_map/command_client_map_object_created.h"

#include <QSqlError>

using namespace server_protocol;

class TaskCreateMapMarker: public TaskDataBase
{
public:
    TaskCreateMapMarker(ActionsClientsManager* clientsManager_,
                        const QString& uuid_client_,
                        const command_server_map_object_create& cmd) :
        // Передаем защищенный SQL-запрос в базовый класс
        TaskDataBase(buildSecureQuery(cmd)),
        data_marker(cmd.getDataMarker()),
        uuid_client(uuid_client_),
        clientsManager(clientsManager_)
    {/* ... */}

    bool processRequestResult(QSqlQuery& query) override final {

        // Поняла ли СУБД наш SQL-запрос
        /// ЗАЩИТА ОТ СБОЕВ СУБД:
        // Проверяем, произошла ли критическая ошибка при выполнении SQL-запроса
        results_requreq res_code = invalid;
        if (query.lastError().isValid()) {
            qWarning() << "TaskRemoveMapMarker: critical error SQL-request!"
                       << query.lastError().text();
        }
        else if (query.next()){
            int code = query.value(0).toInt();

            if (code == 0)
                res_code = successfully;
            else
                res_code = error;
        }

        /// Сообщаем автору команды об результате обработки
        result_command msg_res_command(id_command_server_map_object_create,
                                       res_code);
        emit clientsManager->sendByteArray(uuid_client, msg_res_command.toByteArray());

        /// Если команда выполнена успешно то сообщаем другим клиентам об создании метки
        if (res_code == successfully){

            // Уведомляем другие клиенты об этом, исключая автора
            command_client_map_object_created cmd_obj_created(data_marker);
            emit clientsManager->sendByteArrayAllUsersExcept(QStringList{uuid_client},
                                                             cmd_obj_created.toByteArray());

            qDebug() << "TaskCreateMapMarker: The object was successfully created in the database. The mailing list is completed.";
        }

        // Возвращаем true, если пакет от СУБД в принципе был получен и обработан
        return res_code != invalid;
    }

private:
    // Вспомогательный статический метод для чистой и безопасной сборки SQL-запроса
    static QString buildSecureQuery(const command_server_map_object_create& m) {
        const data_map_marker& marker = m.getDataMarker();

        // Используем стандартную C-локаль, чтобы double ВСЕГДА форматировался с точкой
        QLocale cLocale(QLocale::C);
        QString lonStr = cLocale.toString(marker.lon, 'f', 7);
        QString latStr = cLocale.toString(marker.lat, 'f', 7);

        // Получаем строку иерархии (из QList<uint8_t> делаем "10-25-3")
        QString hierarchyStr = marker.getHierarchyChain_str();

        QString timeStr = marker.lastUpdate.toString(data_map_marker::format_lastUpdate());

        // Используем долларовые кавычки $$ в PostgreSQL.
        // Это полностью защищает от SQL-инъекций, даже если в имени или описании
        // знака будут кавычки, спецсимволы или управляющие хакерские команды.
        return QString("SELECT * FROM __CreateMarkerById("
                       "%1, %2, $$%3$$, $$%4$$, ARRAY[%5, %6, %7], '%8', '%9', '%10'"
                       ");")
            .arg(lonStr)
            .arg(latStr)
            .arg(marker.name)  // Передаем строку как есть, без .replace()
            .arg(marker.info)  // Передаем описание как есть
            .arg(marker.colorName.red())
            .arg(marker.colorName.green())
            .arg(marker.colorName.blue())
            .arg(hierarchyStr)
            .arg(marker.get_uuid())
            .arg(timeStr);
    }

    ActionsClientsManager* clientsManager;
    const QString uuid_client;
    const data_map_marker data_marker;
};

#endif // TASKCREATEMAPMARKER_H

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
                        const command_server_map_object_create& m) :
        // Передаем защищенный SQL-запрос в базовый класс
        TaskDataBase(buildSecureQuery(m)),
        data_marker(m.getDataMarker()),
        login(login_client),
        clientsManager(clientsManager_)
    {/* ... */}

    bool processRequestResult(QSqlQuery& query) override final {
        /// !!!!!!!!!!!!!!!!!
        // Если __CreateMarkerById возвращает код статуса (например, 0 - успех, 1 - ошибка),
        // здесь можно добавить проверку query.next()

        // Сообщаем автору команды что она выполнена успешно
        result_command msg_res_command(id_command_server_map_object_create,
                                       successfully);
        emit clientsManager->sendByteArray(login, msg_res_command.toByteArray());

        // Уведомляем другие клиенты об этом, исключая автора
        command_client_map_object_created cmd_obj_created(data_marker);
        emit clientsManager->sendByteArrayAllUsersExcept(QStringList{login},
                                                         cmd_obj_created.toByteArray());

        qDebug() << "TaskCreateMapMarker: The object was successfully created in the database. The mailing list is completed.";
        return true;
    }

private:
    // Вспомогательный статический метод для чистой и безопасной сборки SQL-запроса
    static QString buildSecureQuery(const command_server_map_object_create& m) {
        const data_map_marker& marker = m.getDataMarker();

        // Используем стандартную C-локаль, чтобы double ВСЕГДА форматировался с точкой (например, 55.751234)
        QLocale cLocale(QLocale::C);
        QString lonStr = cLocale.toString(marker.lon, 'f', 7);
        QString latStr = cLocale.toString(marker.lat, 'f', 7);

        // Защищаем базу данных, удваивая одиночные кавычки во всех пришедших из сети строках
        QString safeUuid = QString(marker.get_uuid()).replace("'", "''");
        QString safeName = QString(marker.name).replace("'", "''");
        QString safeInfo = QString(marker.info).replace("'", "''");
        QString safeTime = QString(marker.lastUpdate.toString(data_map_marker::format_lastUpdate)).replace("'", "''");

        return "SELECT * FROM __CreateMarkerById("
               + lonStr + ","
               + latStr + ","
               + "'" + safeName + "',"
               + "'" + safeInfo + "',"
               + "ARRAY[" + QString::number(marker.colorName.red()) + ","
               + QString::number(marker.colorName.green()) + ","
               + QString::number(marker.colorName.blue()) + "],"
               + "'" + marker.getHierarchyChain_str() + "',"
               + "'" + safeUuid + "',"
               + "'" + safeTime + "'"
               + ");";
    }

    ActionsClientsManager* clientsManager;
    const QString login;
    const data_map_marker data_marker;
};

#endif // TASKCREATEMAPMARKER_H

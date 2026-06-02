#ifndef TASKUPDATEMAPMARKER_H
#define TASKUPDATEMAPMARKER_H

#include "./taskdatabase.h"
#include "../../Network/ActionsClientsManager.h"

#include "../../../common/protocol/commands_server/commands_server_map/command_server_map_object_update.h"
#include "../../../common/protocol/commands_client/commands_client_map/command_client_map_object_update.h"

using namespace server_protocol;

class TaskUpdateMapMarker : public TaskDataBase
{
public:
    TaskUpdateMapMarker(ActionsClientsManager* clientsManager_,
                        const QString& login_client,
                        const command_server_map_object_update& cmd) :
        // Передаем сформированный SQL-запрос в базовый класс, защищая все текстовые поля и вещественные числа
        TaskDataBase(buildSecureQuery(cmd)),
        login(login_client),
        clientsManager(clientsManager_),
        data_marker(cmd.getDataMarker())
    {}

    bool processRequestResult(QSqlQuery& query) override final {
        if (!query.next()) return false;

        int code = query.value(0).toInt();
        switch (code) {
        case 0: {
            qDebug() << "TaskUpdateMapMarker: Успешно обновлено в БД. Рассылаем остальным.";
            command_client_map_object_update cmd_update_marker(data_marker);

            // Уведомляем других об изменениях, кроме инициатора
            emit clientsManager->sendByteArrayAllUsersExcept(QStringList{login},
                                                             cmd_update_marker.toByteArray());
            break;
        }
        case 1:
            qDebug() << "TaskUpdateMapMarker: Ошибка изменения данных метки в БД (code 1)";
            /// Здесь можно сформировать command_client_map_result_requreq_markers с ошибкой и отправить лично автору
            break;

        default:
            qDebug() << "TaskUpdateMapMarker: Получен неизвестный код ответа БД:" << code;
            break;
        }

        return true;
    }

private:
    // Вспомогательный статический метод для чистой и безопасной сборки SQL строки
    static QString buildSecureQuery(const command_server_map_object_update& cmd) {
        const data_map_marker& marker = cmd.getDataMarker();

        // Используем C-локаль (стандартную), чтобы double ВСЕГДА форматировался с точкой (например, 55.751234)
        QLocale cLocale(QLocale::C);
        QString latStr = cLocale.toString(marker.lat, 'f', 7);
        QString lonStr = cLocale.toString(marker.lon, 'f', 7);

        // Экранируем все текстовые поля от инъекций и спецсимволов
        QString safeUuid = QString(marker.get_uuid()).replace("'", "''");
        QString safeName = QString(marker.name).replace("'", "''");
        QString safeInfo = QString(marker.info).replace("'", "''");
        QString safeTime = QString(marker.lastUpdate.toString(data_map_marker::format_lastUpdate)).replace("'", "''");

        return "SELECT * FROM __ChangeInfoMarker('"
               + safeUuid + "',"
               + latStr + ","
               + lonStr + ","
               + "'" + safeTime + "',"
               + "'" + safeName + "',"
               + "'" + safeInfo + "',"
               + "ARRAY[" + QString::number(marker.colorName.red()) + ","
               + QString::number(marker.colorName.green()) + ","
               + QString::number(marker.colorName.blue()) + "],"
               + "'" + QString::number(marker.type_obj_id) + "-" + QString::number(marker.subtype_obj_id) + "'"
               + ");";
    }

    ActionsClientsManager* clientsManager;
    const data_map_marker data_marker;
    const QString login;
};

#endif // TASKUPDATEMAPMARKER_H

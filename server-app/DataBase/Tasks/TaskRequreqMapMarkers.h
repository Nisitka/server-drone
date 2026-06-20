#ifndef TASKREQUREQMAPMARKERS_H
#define TASKREQUREQMAPMARKERS_H

#include "./taskdatabase.h"
#include "../../Network/ActionsClientsManager.h"

#include "../../../common/protocol/commands_client/commands_client_map/command_client_map_requreq_data_markers.h"
#include "../../../common/protocol/commands_client/commands_client_map/command_client_map_result_requreq_markers.h"

using namespace server_protocol;

class TaskRequreqMapMarkers: public TaskDataBase
{
public:
    TaskRequreqMapMarkers(ActionsClientsManager* clientsManager_,
                          const QString& login_client) :
        TaskDataBase("SELECT * FROM __GetInfoMarkers()"),
        login(login_client),
        clientsManager(clientsManager_)
    { }

    bool processRequestResult(QSqlQuery& query) override final
    {
        QVector<data_map_marker> markersList;

        // Извлекаем данные из БД во временный список
        while (query.next())
        {
            QString uuid = query.value(0).toString();

            // Координаты
            double lon = query.value(1).toDouble();
            double lat = query.value(2).toDouble();

            // Имя метки и доп. информация
            QString name = query.value(3).toString();
            QString info = query.value(4).toString();

            // Цвет имени
            uint8_t color_name_r = static_cast<uint8_t>(query.value(5).toUInt());
            uint8_t color_name_g = static_cast<uint8_t>(query.value(6).toUInt());
            uint8_t color_name_b = static_cast<uint8_t>(query.value(7).toUInt());
            QColor color_name(color_name_r, color_name_g, color_name_b);

            // Цепочка вложенности типов метки
            /// uint8_t vlv_types = static_cast<uint8_t>(query.value("lev").toUInt());
            const QString str_hierarchy_chain = query.value("hier").toString();
            QList<uint8_t> hierarchyChain = data_map_marker::parseHierarchyString(str_hierarchy_chain);
            //qDebug() << "TaskRequreqMapMarkers:" << str_hierarchy_chain << hierarchyChain;

            // Обработка даты и времени последнего обновления
            QString text_dateTime = query.value(13).toString();

            // Безопасный chop(3) для отсечения лишних знаков (например, микросекунд БД),
            // защищающий от краша при пустой строке
            if (text_dateTime.size() >= 3) {
                text_dateTime.chop(3);
            }

            QDateTime lastUpdate = QDateTime::fromString(text_dateTime, data_map_marker::format_lastUpdate);

            // Если парсинг времени по формату не удался, ставим текущее время, чтобы не ломать протокол
            if (!lastUpdate.isValid()) {
                lastUpdate = QDateTime::currentDateTime();
            }

            // Наполняем структуру маркера карты
            data_map_marker marker(uuid,
                                   hierarchyChain,
                                   name,
                                   color_name,
                                   lon, lat,
                                   info,
                                   lastUpdate);

            markersList.append(marker);
        }

        // Гарантировано знаем точное количество элементов на любой СУБД (PostgreSQL/SQLite)
        uint32_t totalMarkers = static_cast<uint32_t>(markersList.size());

        // Сообщаем клиенту о начале и количестве отправляемых меток
        // (Мы изменили тип count_markers на uint32_t в прошлых шагах для масштабируемости)
        command_client_map_result_requreq_markers cmd_result(
            successfully,
            totalMarkers
            );
        emit clientsManager->sendByteArray(login, cmd_result.toByteArray());

        // Отправляем сами маркеры один за другим из нашего списка
        for (const data_map_marker& marker: std::as_const(markersList))
        {
            command_client_map_requreq_data_markers cmd(marker);
            emit clientsManager->sendByteArray(login, cmd.toByteArray());
        }

        qDebug() << "TaskRequreqMapMarkers: Успешно отправлено" << totalMarkers << "маркеров пользователю" << login;
        return true;
    }

private:
    ActionsClientsManager* clientsManager;
    const QString login;
};;

#endif // TASKREQUREQMAPMARKERS_H

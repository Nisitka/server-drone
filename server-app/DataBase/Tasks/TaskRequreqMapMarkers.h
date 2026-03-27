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
                          const QString& login_client):
        TaskDataBase("SELECT * FROM __GetInfoMarkers()"),
        login(login_client),
        clientsManager(clientsManager_)
    { /* ... */}

    bool processRequestResult(QSqlQuery& query) override final
    {

        int count = query.size();
        if (count < 0){
            command_client_map_result_requreq_markers cmd_result(command_client_map_result_requreq_markers::invalid, 0);
            emit clientsManager->sendByteArray(login, cmd_result.toByteArray());
            return false;
        }

        // Сообщаем о начале отправки меток
        command_client_map_result_requreq_markers cmd_result(command_client_map_result_requreq_markers::successfully, count);
        emit clientsManager->sendByteArray(login, cmd_result.toByteArray());

        /// Извлекаем данные и отправляем клиенту
        while (query.next())
        {
            QString uuid = query.value(0).toString();

            // Координаты
            double lat = query.value(2).toDouble();
            double lon = query.value(1).toDouble();

            // Имя метки
            const QString name = query.value(3).toString();

            // Доп. информация
            const QString info = query.value(4).toString();

            // Цвет имени
            uint8_t color_name_r = query.value(5).toUInt();
            uint8_t color_name_g = query.value(6).toUInt();
            uint8_t color_name_b = query.value(7).toUInt();
            //qDebug() << color_name_r << color_name_g << color_name_b;
            QColor color_name(color_name_r, color_name_g, color_name_b);

            // Название типа метки
            const QString name_subtype = query.value(8).toString();

            // Тип и подтип метки
            uint8_t id_subtype = query.value(9).toUInt();
            uint8_t lvl_parent = query.value(10).toUInt();
            uint8_t id_type = query.value(11).toUInt();
            const QString heir_types = query.value(12).toString();

            QString text_dateTime = query.value(13).toString();
            text_dateTime.chop(3);

            QDateTime lastUpdate = QDateTime::fromString(text_dateTime, data_map_marker::format_lastUpdate);

            qDebug() << uuid << text_dateTime << lastUpdate << id_type << id_subtype << name << color_name <<
                        lat << lon << info;

            // Заполянем данными и создаем команду на основе их
            data_map_marker marker(uuid,
                                   id_type, id_subtype,
                                   name,
                                   color_name,
                                   lat, lon,
                                   info,
                                   lastUpdate);
            command_client_map_requreq_data_markers cmd(marker);

            emit clientsManager->sendByteArray(login, cmd.toByteArray());
        }

        return true;
    }

private:
    ActionsClientsManager* clientsManager;
    const QString login;
};

#endif // TASKREQUREQMAPMARKERS_H

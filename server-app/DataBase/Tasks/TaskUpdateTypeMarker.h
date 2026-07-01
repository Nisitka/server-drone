#ifndef TASKUPDATETYPEMARKER_H
#define TASKUPDATETYPEMARKER_H

#pragma once

#include "./taskdatabase.h"
#include "../../Network/ActionsClientsManager.h"
#include "../../../common/protocol/commands_server/commands_server_map/command_server_map_update_type_markers.h"
#include "../../../common/protocol/commands_client/commands_client_map/command_client_map_result_requreq_type_markers.h"

#include <QSqlError>
#include <QDebug>

using namespace server_protocol;

class TaskUpdateTypeMarker: public TaskDataBase
{
public:
    TaskUpdateTypeMarker(ActionsClientsManager* clientsManager_,
                         const QString& uuid_client_,
                         const command_server_map_update_type_markers& cmd) :
        // Передаем сформированный безопасный SQL-запрос обновления в базовый класс
        TaskDataBase(buildQuery(cmd)),
        clientsManager(clientsManager_),
        uuid_client(uuid_client_),
        type_record(cmd.get_type_marker())
    { }

    bool processRequestResult(QSqlQuery& query) override final
    {
        // Проверяем системные ошибки СУБД (например, потеря связи с PostgreSQL)
        if (query.lastError().isValid()) {
            qWarning() << "TaskUpdateTypeMarker: Critical database error when updating the type!"
                       << query.lastError().text();

            // Отправляем клиенту-автору статус invalid (системный сбой)
            result_command msg_res_command(id_command_server_map_update_type_markers, invalid);
            emit clientsManager->sendByteArray(uuid_client, msg_res_command.toByteArray());
            return false;
        }

        // Разбираем код ответа от хранимой процедуры СУБД
        results_requreq res_code = invalid;
        if (query.next()) {
            int code = query.value(0).toInt();

            if (code == 0) {
                res_code = successfully;
            } else {
                res_code = error; // Ошибка бизнес-логики (например, обновляемый тип не найден в таблице)
            }
        } else {
            qWarning() << "TaskUpdateTypeMarker: database return empty result!";
            res_code = error;
        }

        // Отправляем автору команды результат её выполнения
        result_command msg_res_command(id_command_server_map_update_type_markers, res_code);
        emit clientsManager->sendByteArray(uuid_client, msg_res_command.toByteArray());

        if (res_code == successfully) {
            qDebug() << "TaskUpdateTypeMarker: Данные типа меток успешно обновлены в БД:"
                     << type_record.name << type_record.hierarchy_chain;

            // Тип, который только что создали
            QList<data_type_marker_record> changed_type = { type_record };

            // На какой момент времени будет отправлено состояние типов
            QDateTime date_time = QDateTime::currentDateTime();
            results_requreq result = successfully;

            // По какой причине был запрос типов
            command_client_map_result_requreq_type_markers::motive m_motive = command_client_map_result_requreq_type_markers::update;

            command_client_map_result_requreq_type_markers cmd(result, m_motive, date_time, changed_type, QList<QList<uint8_t>>{});
            emit clientsManager->sendByteArrayAllUsersExcept(QStringList{}, cmd.toByteArray());
        }

        return res_code != invalid;
    }

private:
    // Вспомогательный статический метод для сборки и тотальной защиты SQL-запроса
    static QString buildQuery(const command_server_map_update_type_markers& cmd) {
        const data_type_marker_record& record = cmd.get_type_marker();

        // Превращаем цепочку QList<uint8_t> в строку иерархии через дефисы ("10-25-3")
        QString hierarchyStr = data_map_marker::makeHierarchyString(record.hierarchy_chain);

        /// УПАКОВКА БИНАРНЫХ ДАННЫХ ДЛЯ bytea:
        // Переводим измененный массив картинки в HEX-формат для безопасной вставки в текст SQL.
        QString hexIconData = QString::fromLatin1(record.iconBytes.toHex());
        QString sqlByteaValue = "E'\\\\x" + hexIconData + "'";

        // Имя защищаем долларовыми кавычками $$ на случай наличия кавычек в тексте
        return QString("SELECT * FROM __UpdateMarkerType("
                       "'%1', $$%2$$, %3"
                       ");")
            .arg(hierarchyStr)
            .arg(record.name)
            .arg(sqlByteaValue);
    }

    ActionsClientsManager* clientsManager;
    const QString uuid_client;
    const data_type_marker_record type_record;
};


#endif // TASKUPDATETYPEMARKER_H

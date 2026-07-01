#ifndef TASKCREATETYPEMARKER_H
#define TASKCREATETYPEMARKER_H

#include "./taskdatabase.h"
#include "../../Network/ActionsClientsManager.h"
#include "../../../common/protocol/commands_server/commands_server_map/command_server_map_create_type_markers.h"
#include "../../../common/protocol/commands_client/commands_client_map/command_client_map_result_requreq_type_markers.h"

#include <QSqlError>
#include <QDebug>

using namespace server_protocol;

class TaskCreateTypeMarker: public TaskDataBase
{
public:
    TaskCreateTypeMarker(ActionsClientsManager* clientsManager_,
                         const QString& uuid_client_,
                         const command_server_map_create_type_markers& cmd) :
        // Передаем сформированный безопасный SQL-запрос в базовый класс
        TaskDataBase(buildQuery(cmd)),
        clientsManager(clientsManager_),
        uuid_client(uuid_client_),
        type_record(cmd.get_type_marker())
    { }

    bool processRequestResult(QSqlQuery& query) override final
    {
        // Проверяем, не упала ли СУБД из-за системной ошибки
        if (query.lastError().isValid()) {
            qWarning() << "TaskCreateTypeMarker: A critical DBMS error when creating a type!"
                       << query.lastError().text();

            // Отправляем автору статус ошибки пакета/сервера (invalid)
            result_command msg_res_command(id_command_server_map_create_type_markers, invalid);
            emit clientsManager->sendByteArray(uuid_client, msg_res_command.toByteArray());
            return false;
        }

        // Разбираем ответ от хранимой процедуры PostgreSQL
        results_requreq res_code = invalid;
        if (query.next()) {
            int code = query.value(0).toInt();

            if (code == 0) {
                res_code = successfully;
            } else {
                res_code = error; // Ошибка бизнес-логики (например, такой тип уже существует)
            }
        } else {
            qWarning() << "TaskCreateTypeMarker: postgres return empty result!";
            res_code = error;
        }

        // Отправляем автору команды результат её выполнения
        result_command msg_res_command(id_command_server_map_create_type_markers, res_code);
        emit clientsManager->sendByteArray(uuid_client, msg_res_command.toByteArray());

        if (res_code == successfully) {
            qDebug() << "TaskCreateTypeMarker: A new type of tags has been successfully created in the database:"
                     << type_record.name << type_record.hierarchy_chain;

            /// !!!!
            /// Нужно ли сообщать об этом клиентам?!

            // Тип, который только что создали
            QList<data_type_marker_record> created_type = { type_record };

            // На какой момент времени будет отправлено состояние типов
            QDateTime date_time = QDateTime::currentDateTime();
            results_requreq result = successfully;

            // По какой причине был запрос типов
            command_client_map_result_requreq_type_markers::motive m_motive = command_client_map_result_requreq_type_markers::update;

            command_client_map_result_requreq_type_markers cmd(result, m_motive, date_time, created_type, QList<QList<uint8_t>>{});
            emit clientsManager->sendByteArrayAllUsersExcept(QStringList{}, cmd.toByteArray());
        }

        return res_code != invalid;
    }

private:
    // Вспомогательный статический метод для сборки и тотальной защиты SQL-запроса
    static QString buildQuery(const command_server_map_create_type_markers& cmd) {
        const data_type_marker_record& record = cmd.get_type_marker();

        // Строка иерархии через дефисы (например, "10-25-3")
        QString hierarchyStr = data_map_marker::makeHierarchyString(record.hierarchy_chain);

        /// ПАКОВКА БИНАРНЫХ ДАННЫХ:
        // Чтобы передать bytea (картинку) внутри текстового SQL-запроса без bindValue,
        // мы переводим массив в HEX-формат. Префикс E'\\x...' указывает PostgreSQL,
        // что это шестнадцатеричный бинарный поток, и СУБД сама рампакует его обратно в bytea.
        QString hexIconData = QString::fromLatin1(record.iconBytes.toHex());
        QString sqlByteaValue = "E'\\\\x" + hexIconData + "'";

        // Текстовое поле имени защищаем долларовыми кавычками $$
        return QString("SELECT * FROM __CreateMarkerType("
                       "'%1', $$%2$$, %3"
                       ");")
            .arg(hierarchyStr)
            .arg(record.name)
            .arg(sqlByteaValue); // Передаем сформированную HEX-строку bytea
    }

    ActionsClientsManager* clientsManager;
    const QString uuid_client;
    const data_type_marker_record type_record;
};


#endif // TASKCREATETYPEMARKER_H

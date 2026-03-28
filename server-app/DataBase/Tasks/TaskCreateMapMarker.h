#ifndef TASKCREATEMAPMARKER_H
#define TASKCREATEMAPMARKER_H

#include "./taskdatabase.h"

#include "../../../common/protocol/commands_server/commands_server_map/command_server_map_object_create.h"

using namespace server_protocol;

class TaskCreateMapMarker: public TaskDataBase
{
public:
    TaskCreateMapMarker(const command_server_map_object_create& m):
    TaskDataBase("SELECT __CreateMarkerById("
        + QString::number(m.getDataMarker().lat) + ","
        + QString::number(m.getDataMarker().lon) + ","

        + "'" + m.getDataMarker().name + "',"
        + "'" + m.getDataMarker().info + "',"

        + "ARRAY["  + QString::number(m.getDataMarker().colorName.red()) + ","
        + QString::number(m.getDataMarker().colorName.green()) + ","
        + QString::number(m.getDataMarker().colorName.blue()) + "]::smallint[],"

        + "'" + QString::number(m.getDataMarker().type_obj_id) + "-"
        + QString::number(m.getDataMarker().subtype_obj_id) + "',"

        + "'" + m.getDataMarker().uuid + "',"

        + "'" + m.getDataMarker().lastUpdate.toString() + "',"

        + ");")
    {/* ... */}

    bool processRequestResult(QSqlQuery& query) override final{

        return true;
    }
};

#endif // TASKCREATEMAPMARKER_H

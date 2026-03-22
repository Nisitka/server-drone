#ifndef TASKCREATEMAPMARKER_H
#define TASKCREATEMAPMARKER_H

#include "./taskdatabase.h"

#include "../../../common/protocol/commands_server/commands_server_map/command_server_map_object_create.h"

using namespace server_protocol;

class TaskCreateMapMarker: public TaskDataBase
{
public:
    TaskCreateMapMarker(const command_server_map_object_create& m):
        TaskDataBase("SELECT __CreateMarker(" + QString::number(m.Lat()) + ","
                                              + QString::number(m.Lon()) + ","
                                              + QString::number(m.type_object()) + ","
                                              + QString::number(m.subtype_object()) + ","
                                              + "'" + m.Name() + "',"
                                              + "'" + m.Info() + "',"
    + "ARRAY["  + QString::number(m.ColorName_R()) + ","
                + QString::number(m.ColorName_G()) + ","
                + QString::number(m.ColorName_B()) + "]::smallint[],"
    + ");")
    {/* ... */}

    bool processRequestResult(QSqlQuery& query) override final{

        return true;
    }
};

#endif // TASKCREATEMAPMARKER_H

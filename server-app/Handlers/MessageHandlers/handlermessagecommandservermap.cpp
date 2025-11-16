#include "handlermessagecommandservermap.h"

HandlerMessageCommandServerMap::HandlerMessageCommandServerMap():
    HandlerMessage(server_protocol::id_msg_command_server_map)
{

}

bool HandlerMessageCommandServerMap::processingMessage(QByteArray &data)
{


    return true;
}

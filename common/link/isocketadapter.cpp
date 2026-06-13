#include "isocketadapter.h"


ISocketAdapter::ISocketAdapter(): QObject(),
    lastMsgId(server_protocol::id_msg_unknown) {

    connect(this, &ISocketAdapter::trSendByteArray,
            this, &ISocketAdapter::sendByteArray);
}

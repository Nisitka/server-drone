#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include "./protocol_message.h"

namespace server_protocol {

inline const int HEARTBEAT_INTERVAL = 5000; // ms

class heartbeat: public protocol_message{

public:

    heartbeat():
        protocol_message(id_msg_heartbeat)
    { /* ... */}
};

}

#endif // HEARTBEAT_H

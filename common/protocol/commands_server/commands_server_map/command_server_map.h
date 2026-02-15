#ifndef COMMAND_SERVER_MAP_H
#define COMMAND_SERVER_MAP_H

#include <cstdint>

namespace server_protocol {

enum type_object_map: uint8_t{
    drone,
    marker
};

}

#endif // COMMAND_SERVER_MAP_H

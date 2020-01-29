//
// Created by przemek on 20.11.2019.
//

#ifndef SIMPLE_P2P_GENERALTYPES_H
#define SIMPLE_P2P_GENERALTYPES_H

#include <cinttypes>

#define Uint8  uint8_t
#define Uint16 uint16_t
#define Uint32 uint32_t
#define Uint64 uint64_t

#define Int8 int8_t
#define Int16 int16_t
#define Int32 int32_t
#define Int64 int64_t

#define SegmentId Uint16

#define QUIT_CONN 4
#define REQ_SEGMENT 8
#define REVOKE 16
#define FILE_LIST 32

#define BEACON_MAX_COUNT 3

#define RESOURCE_HEADER_SIZE 264

#define UDP_SERV_BUFFER_SIZE (sizeof(Uint8)+sizeof(Uint64)+(BEACON_MAX_COUNT*RESOURCE_HEADER_SIZE))

#define BROADCAST_ADDRESS "192.168.1.255"
#define BROADCAST_PORT 2020

#define TCP_SERVER_PORT 2169

#define TIMEOUT_COUNTER_LIMIT 20
#define LEFT_MARGIN TIMEOUT_COUNTER_LIMIT
#define MAX_TIMEOUT 100ms
#define BAN_TIME 1000ms
#define TIMEOUT_CHECK_INTERVAL 300ms

namespace simpleP2P {
const Uint16 FILE_NAME_LENGHT = 256;
const Uint16 FILE_SIZE_LENGHT = 8;
const Uint16 SEGMENT_SIZE = 1024;
}
#endif //SIMPLE_P2P_GENERALTYPES_H

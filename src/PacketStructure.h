#pragma once

#include <cstdint>


#define REAP_VERSION 001
#define MAX_BUFFER_SIZE 65536

// All Packets are Bi-Directional except for ErrorReport

enum OperationType_t : uint16_t
{
    PING = 0,
    OPENPROCESS = 1,
    WRITEPROCESSMEMORY = 2,
    READPROCESSMEMORY = 3,
    ERRORREPORT = 4,
};

#define CASE_STRING( x ) case static_cast<int>( x ) : return #x

static inline const char* Op2String(OperationType_t op) {
    switch (op) {
        CASE_STRING(PING);
        CASE_STRING(OPENPROCESS);
        CASE_STRING(WRITEPROCESSMEMORY);
        CASE_STRING(READPROCESSMEMORY);
        CASE_STRING(ERRORREPORT);
        default:
            return "UNKNOWN";
    }
}
struct ReapPacketHeader
{
    //uint8_t magic[5] = "reap";
    //char magic[5] = "reap";
    uint8_t magic = 0x666;
    uint8_t version = REAP_VERSION;
    OperationType_t type;
};

struct ReapErrorReport : ReapPacketHeader
{
    ReapErrorReport() {
        this->type = OperationType_t::ERRORREPORT;
    }
    OperationType_t errorType;
    char errorString[64];
};

struct ReapMemoryRequest : ReapPacketHeader
{
    uint64_t startAddr;
    uint32_t totalBytesManipulated;
};

struct ReapWriteRequest : ReapMemoryRequest
{
    ReapWriteRequest() {
        this->type = OperationType_t::WRITEPROCESSMEMORY;
    }
    uint8_t buffer[MAX_BUFFER_SIZE]; // should be dynamic, make sure this is at the end and we'll just cut if off
};

struct ReapReadRequest : ReapMemoryRequest
{
    uint8_t buffer[MAX_BUFFER_SIZE]; // should be dynamic, make sure this is at the end and we'll just cut if off
};


// Process Setup
struct ReapOpenProcessRequest : ReapPacketHeader
{
    char processName[16]; // gets cut off at 15, this is currently a limitation of vmread/windows kernel
};

// The biggest possible.
struct ReapRequestGeneric : ReapReadRequest
{

};
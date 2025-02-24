// coordinate.h
#pragma once
#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
    float x;        
    float y;        
    uint32_t seq;   
} CoordinatePacket;
#pragma pack(pop)

static_assert(sizeof(CoordinatePacket) == 12, "Invalid struct size");
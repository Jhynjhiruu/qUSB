#ifndef _REMAP_H
#define _REMAP_H

#include <ultra64.h>

#define MAX_REMAP_INSTRS (16)

typedef struct {
    s8 x;
    s8 y;
} __attribute__((packed)) StickData;

extern u8 passthrough[][MAX_REMAP_INSTRS];

StickData parse_remap(void *out, u8 bytes, u8 (*mappings)[MAX_REMAP_INSTRS]);
void remap_init(const char *filename);

#endif
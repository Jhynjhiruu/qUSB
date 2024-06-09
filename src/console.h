#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "video.h"

void print_str(FB_TYPE col, size_t x, size_t y, const char *str);
void print(const char *str);
void print_u32(u32 num);
void console_clear(void);
void console_init(void);

#endif
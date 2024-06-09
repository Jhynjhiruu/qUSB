#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include <ultra64.h>

typedef struct {
    union {
        struct {
            unsigned int a : 1;
            unsigned int b : 1;
            unsigned int z : 1;
            unsigned int start : 1;
            unsigned int d_up : 1;
            unsigned int d_down : 1;
            unsigned int d_left : 1;
            unsigned int d_right : 1;
            unsigned int pad0 : 1;
            unsigned int pad1 : 1;
            unsigned int l : 1;
            unsigned int r : 1;
            unsigned int c_up : 1;
            unsigned int c_down : 1;
            unsigned int c_left : 1;
            unsigned int c_right : 1;
        };
        u16 button;
    };
    s8 stick_x;
    s8 stick_y;
    u8 err;
} Controller;

extern Controller controller_data[MAXCONTROLLERS];

u32 controller_init(void);
void read_controllers(void);

#endif
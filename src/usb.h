#ifndef _USB_H
#define _USB_H

#include <ultra64.h>

typedef enum {
    XINPUT,
    POKKEN,
    N64,
    MASS,
} Interface;

void hook_usb(Interface interface);

#endif
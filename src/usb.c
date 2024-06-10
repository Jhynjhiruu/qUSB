#include <ultra64.h>

#include "usb.h"

typedef struct {
    void (*reset_endpoint)(OSBbUsbHandle, u32, u8, u8 *, u32);
    void (*get_desc)(OSBbUsbHandle, u32, setup_struct *);
    void (*vendor)(OSBbUsbHandle, u32, setup_struct *);
    void (*init_endpoints)(OSBbUsbHandle);
    void (*query)(OSBbUsbInfo *);
    void (*stall_endpoint)(OSBbUsbHandle, u8, u32);
} device_funcs;

void hook_usb(Interface interface) {}
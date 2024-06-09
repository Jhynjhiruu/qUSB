#include <macros.h>
#include <ultra64.h>

#include "console.h"
#include "controller.h"
#include "queue.h"

static OSContStatus controller_status[MAXCONTROLLERS];
Controller controller_data[MAXCONTROLLERS];

u32 controller_init(void) {
    u8 attached;
    u32 num_controllers = 0;

    osCreateMesgQueue(&controller_mesg_queue, &vi_retrace_mesg, 1);
    osSetEventMesg(OS_EVENT_SI, &controller_mesg_queue, (OSMesg)0);

    osContInit(&controller_mesg_queue, &attached, controller_status);

    for (u32 i = 0; i < MAXCONTROLLERS; i++) {
        if ((attached & (1 << i)) &&
            ((controller_status[i].errno & CONT_NO_RESPONSE_ERROR) == 0)) {
            num_controllers++;
        }
    }

    print("Controllers initialised");

    return num_controllers;
}

void read_controllers(void) {

    osRecvMesg(&controller_mesg_queue, &vi_retrace_mesg, OS_MESG_BLOCK);
    osContGetReadData((OSContPad *)controller_data);
}

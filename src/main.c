#include <macros.h>
#include <ultra64.h>

#include "console.h"
#include "controller.h"
#include "menu.h"
#include "queue.h"
#include "remap.h"
#include "stack.h"
#include "video.h"

void __osBbVideoPllInit(s32);
void osBbPowerOff(void);
void osBbSetErrorLed(u32);

s32 osBbUsbInit(void);

void __osBbDelay(u32);

u8 bootStack[STACK_SIZE] __attribute__((aligned(STACK_ALIGN)));

OSThread idlethread;
void idleproc(void *);
u8 idlestack[STACK_SIZE] __attribute__((aligned(STACK_ALIGN)));

OSThread mainthread;
void mainproc(void *);
u8 mainstack[STACK_SIZE] __attribute__((aligned(STACK_ALIGN)));

#define PRESSED(key) ((change & (key)) && (status & (key)))

void boot(void) {
#ifdef BBPLAYER
    __osBbVideoPllInit(OS_TV_NTSC);
#endif

    osInitialize();

    osCreateThread(&idlethread, 1, idleproc, NULL,
                   idlestack + sizeof(idlestack), 20);
    osStartThread(&idlethread);
}

void idleproc(void *argv) {
    video_setup();

    osCreatePiManager(OS_PRIORITY_PIMGR, &pi_mesg_queue, pi_mesg_buf,
                      ARRLEN(pi_mesg_buf));

    osCreateMesgQueue(&si_mesg_queue, si_mesg_buf, ARRLEN(si_mesg_buf));
    osSetEventMesg(OS_EVENT_SI, &si_mesg_queue, (OSMesg)ARRLEN(si_mesg_buf));

    osCreateThread(&mainthread, 3, mainproc, argv,
                   mainstack + sizeof(mainstack), 18);
    osStartThread(&mainthread);

    osSetThreadPri(NULL, OS_PRIORITY_IDLE);

    while (TRUE)
        ;
}

void mainproc(void *argv) {
    OSMesg msg;

    osCreateMesgQueue(&vi_mesg_queue, vi_mesg_buf, ARRLEN(vi_mesg_buf));
    osViSetEvent(&vi_mesg_queue, (OSMesg)OS_EVENT_VI, 1);

    video_init();

    osRecvMesg(&vi_mesg_queue, &msg, OS_MESG_BLOCK);
    osRecvMesg(&vi_mesg_queue, &msg, OS_MESG_BLOCK);

    console_init();

    print("remap start");

    remap_init("mapping.bin");

    controller_init();

    while (TRUE) {
        osRecvMesg(&vi_mesg_queue, &msg, OS_MESG_BLOCK);
        menu();
    }
}
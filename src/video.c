#include <ultra64.h>

#include "video.h"

static FB_TYPE framebuffer_a[WIDTH * HEIGHT]
    __attribute__((aligned(FRAMEBUF_ALIGN))) = {0};
static FB_TYPE framebuffer_b[WIDTH * HEIGHT]
    __attribute__((aligned(FRAMEBUF_ALIGN))) = {0};

void video_setup(void) {
    osCreateViManager(OS_PRIORITY_VIMGR);
    osViSetMode(&MODE);
    osViBlack(TRUE);
}

void video_init(void) {
    osViBlack(FALSE);
    osViSwapBuffer(framebuffer_a);
    osViSwapBuffer(framebuffer_b);

    osWritebackDCacheAll();
}
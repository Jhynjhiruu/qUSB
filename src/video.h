#ifndef _VIDEO_H
#define _VIDEO_H

#define WIDTH (320)
#define HEIGHT (240)
#define FRAMEBUF_ALIGN (8)

#define BITS (16)

#if BITS == 16

#define FB_TYPE u16
#define MODE osViModeNtscLpn1

#define WHITE (0xFFFF)
#define BLACK (0x0000)
#define RED (0xF800)
#define GREEN (0x07C0)
#define BLUE (0x003F)

#else

#define FB_TYPE u32
#define MODE osViModeNtscLpn2

#define WHITE (0xFFFFFFFF)
#define BLACK (0x00000000)
#define RED (0xFF000000)
#define GREEN (0x00FF0000)
#define BLUE (0x0000FF00)

#endif

void video_setup(void);
void video_init(void);

#endif
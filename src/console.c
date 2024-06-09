#include <ultra64.h>

#include <string.h>

#include "font.h"
#include "video.h"

#define _ROWS (HEIGHT / CHAR_HEIGHT)
#define _COLUMNS (WIDTH / CHAR_WIDTH)

#define OVERSCAN_V (3)
#define OVERSCAN_H (3)

#define ROWS (_ROWS - 2 * OVERSCAN_V)
#define COLUMNS (_COLUMNS - 2 * OVERSCAN_H)

char lines[_ROWS][_COLUMNS];

static FB_TYPE screenbuf[WIDTH * HEIGHT];

static void move_up(u8 lines) {
    bcopy(&screenbuf[lines * WIDTH * CHAR_HEIGHT], screenbuf,
          WIDTH * (HEIGHT - lines * CHAR_HEIGHT) * sizeof(FB_TYPE));
    bzero(&screenbuf[WIDTH * (HEIGHT - lines * CHAR_HEIGHT)],
          lines * WIDTH * CHAR_HEIGHT * sizeof(FB_TYPE));
}

static void sync_framebuf(void) {
    void *fb = osViGetNextFramebuffer();
    bcopy(screenbuf, fb, WIDTH * HEIGHT * sizeof(FB_TYPE));
}

static void print_char(FB_TYPE colour, size_t x, size_t y, int c) {
    if ((x >= COLUMNS) || (y >= ROWS)) {
        return;
    }

    c -= ' ';

    for (size_t y_off = 0; y_off < CHAR_HEIGHT; y_off++) {
        for (size_t x_off = 0; x_off < CHAR_WIDTH; x_off++) {
            if (font[c][y_off] & (1 << (7 - x_off))) {
                screenbuf[((y + OVERSCAN_V) * CHAR_HEIGHT + y_off) * WIDTH +
                          (x + OVERSCAN_H) * CHAR_WIDTH + x_off] = colour;
            }
        }
    }
}

void print_str(FB_TYPE col, size_t x, size_t y, const char *str) {
    for (size_t i = 0, column = x, row = y; str[i] != '\0'; i++) {
        const char c = str[i];

        if (c == '\n') {
            column = 0;
            row++;
        } else {
            print_char(col, column, row, str[i]);
            column++;
        }
        if (column >= COLUMNS) {
            column = 0;
            row++;
        }
    }

    osWritebackDCache(screenbuf, sizeof(screenbuf));
    sync_framebuf();
}

void print(const char *str) {
    const size_t len = strlen(str);
    u8 uses_lines = 1;

    for (size_t i = 0, column = 0; i < len; i++) {
        const char c = str[i];
        column++;
        if ((c == '\n') || (column >= COLUMNS)) {
            column = 0;
            uses_lines++;
        }
    }

    move_up(uses_lines);

    print_str(WHITE, 0, ROWS - uses_lines, str);
}

void print_u32(u32 num) {
    char buf[8 + 1]; // null terminator
    for (size_t i = 0; i < 8; i++) {
        u8 nibble = (num >> (28 - 4 * i)) & 0x0F;
        if (nibble < 10) {
            buf[i] = nibble + '0';
        } else {
            buf[i] = nibble - 10 + 'A';
        }
    }
    buf[8] = 0;
    print(buf);
}

void console_clear(void) {
    bzero(lines, _ROWS * _COLUMNS);
    bzero(screenbuf, WIDTH * HEIGHT * sizeof(FB_TYPE));
}

void console_init(void) {
    console_clear();

    print("Console init");
}
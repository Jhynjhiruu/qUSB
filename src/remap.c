#include <PR/bbfs.h>
#include <macros.h>
#include <ultra64.h>

#include "console.h"
#include "controller.h"
#include "remap.h"

#define NUM_BUTTONS (18)

enum MappingInstruction {
    PUSH_A,
    PUSH_B,
    PUSH_Z,
    PUSH_L,
    PUSH_R,
    PUSH_START,
    PUSH_C_UP,
    PUSH_C_DOWN,
    PUSH_C_LEFT,
    PUSH_C_RIGHT,
    PUSH_D_UP,
    PUSH_D_DOWN,
    PUSH_D_LEFT,
    PUSH_D_RIGHT,
    PUSH_ANA_UP,
    PUSH_ANA_DOWN,
    PUSH_ANA_LEFT,
    PUSH_ANA_RIGHT,

    AND = 0x80,
    OR,
    NOT,
    XOR,

    PUSH_TRUE = 0x84
};

u8 passthrough[][MAX_REMAP_INSTRS] = {
    {
        1,
        PUSH_A,
    },
    {
        1,
        PUSH_B,
    },
    {
        1,
        PUSH_Z,
    },
    {
        1,
        PUSH_L,
    },
    {
        1,
        PUSH_R,
    },
    {
        1,
        PUSH_START,
    },
    {
        1,
        PUSH_C_UP,
    },
    {
        1,
        PUSH_C_DOWN,
    },
    {
        1,
        PUSH_C_LEFT,
    },
    {
        1,
        PUSH_C_RIGHT,
    },
    {
        1,
        PUSH_D_UP,
    },
    {
        1,
        PUSH_D_DOWN,
    },
    {
        1,
        PUSH_D_LEFT,
    },
    {
        1,
        PUSH_D_RIGHT,
    },
    {
        1,
        PUSH_ANA_UP,
    },
    {
        1,
        PUSH_ANA_DOWN,
    },
    {
        1,
        PUSH_ANA_LEFT,
    },
    {
        1,
        PUSH_ANA_RIGHT,
    },
};

u8 (*xinput_mapping)[MAX_REMAP_INSTRS] = NULL;
u8 (*pokken_mapping)[MAX_REMAP_INSTRS] = NULL;
u8 (*n64_mapping)[MAX_REMAP_INSTRS] = NULL;

static u8 stack[MAX_REMAP_INSTRS];

static unsigned int stack_ptr;

u8 buttons[NUM_BUTTONS];

static void reset_remap(void) {
    bzero(stack, sizeof(stack));
    stack_ptr = 0;
}

static void push_stack(u8 value) {
    if (stack_ptr < MAX_REMAP_INSTRS) {
        stack[stack_ptr++] = value;
    }
}

static u8 pop_stack(void) {
    if (stack_ptr > 0) {
        return stack[--stack_ptr];
    }
    return FALSE;
}

#define BINARY_OP(op)                                                          \
    const u8 a = pop_stack();                                                  \
    const u8 b = pop_stack();                                                  \
    push_stack(a op b)

static u8 do_remap(const u8 *const mapping) {
    const size_t size = MIN(mapping[0], MAX_REMAP_INSTRS - 1);

    reset_remap();

    for (size_t i = 0; i < size; i++) {
        const u8 instr = mapping[i + 1];
        if (instr < 0x80) { // push button
            if (instr < NUM_BUTTONS) {
                push_stack(buttons[instr]);
            }
        } else {
            switch (instr) {
                case AND:
                    {
                        BINARY_OP(&);
                        break;
                    }
                case OR:
                    {
                        BINARY_OP(|);
                        break;
                    }
                case NOT:
                    {
                        push_stack(!pop_stack());
                        break;
                    }
                case XOR:
                    {
                        BINARY_OP(^);
                        break;
                    }
                case PUSH_TRUE:
                    {
                        push_stack(TRUE);
                        break;
                    }
            }
        }
    }

    // assume the return value is at the bottom of the stack
    return stack[0];
}

StickData parse_remap(void *out, u8 bytes, u8 (*mappings)[MAX_REMAP_INSTRS]) {
    u8 *const buf = out;

    int xPos, yPos;
    static int maxX = 60;
    static int minX = -60;
    static int maxY = 60;
    static int minY = -60;

    StickData rv;

    read_controllers();

    xPos = controller_data[0].stick_x;
    yPos = controller_data[0].stick_y;

    maxX = xPos > maxX ? xPos : maxX;
    minX = xPos < minX ? xPos : minX;

    rv.x = (xPos > 0) ? (xPos * 127 / maxX) : (xPos * -128 / minX);

    maxY = yPos > maxY ? yPos : maxY;
    minY = yPos < minY ? yPos : minY;

    rv.y = (yPos > 0) ? (yPos * 127 / maxY) : (yPos * -128 / minY);

    buttons[0] = controller_data[0].a;
    buttons[1] = controller_data[0].b;
    buttons[2] = controller_data[0].z;
    buttons[3] = controller_data[0].l;
    buttons[4] = controller_data[0].r;
    buttons[5] = controller_data[0].start;

    buttons[6] = controller_data[0].c_up;
    buttons[7] = controller_data[0].c_down;
    buttons[8] = controller_data[0].c_left;
    buttons[9] = controller_data[0].c_right;

    buttons[10] = controller_data[0].d_up;
    buttons[11] = controller_data[0].d_down;
    buttons[12] = controller_data[0].d_left;
    buttons[13] = controller_data[0].d_right;

    buttons[14] = rv.y > 63;
    buttons[15] = rv.y < -64;
    buttons[16] = rv.x < -64;
    buttons[17] = rv.x > 63;

    for (size_t i = 0; i < (bytes * 8); i++) {
        buf[i / 8] &= ~(0x80 >> (i & 7));
        if (do_remap(mappings[i])) {
            buf[i / 8] |= (0x80 >> (i & 7));
        }
    }

    return rv;
}

#ifdef BBPLAYER
static u8 block[1024 * 16];
static OSBbFs fs;
static OSBbStatBuf sb;
static s32 fd;
#endif

void remap_init(const char *filename) {
#ifdef BBPLAYER
    print("osBbFInit");
    osBbFInit(&fs);

    print("osBbFOpen");
    print(filename);
    fd = osBbFOpen(filename, "rb");
    if (fd == BBFS_ERR_ENTRY) {
        if (osBbFCreate(filename, 1, sizeof(block)) != 0) {
            print("Failed to create mapping file");
            return;
        }
        fd = osBbFOpen(filename, "rb");
    }

    print("osBbFStat");
    osBbFStat(fd, &sb, NULL, 0);
    if (sb.size != sizeof(block)) {
        print("Error loading mapping file");
        print_u32(sb.size);
        osBbFClose(fd);
    }

    print("osBbFRead");
    osBbFRead(fd, 0, block, sizeof(block));
    print("osBbFClose");
    osBbFClose(fd);

    if (bcmp("MAPS", block, 4)) {
        print("Invalid data");
        return;
    }

    xinput_mapping = (u8(*)[MAX_REMAP_INSTRS]) & block[8];

    pokken_mapping = xinput_mapping + block[4];

    n64_mapping = pokken_mapping + block[5];
#else
    xinput_mapping = passthrough;

    pokken_mapping = passthrough;

    n64_mapping = passthrough;
#endif
}
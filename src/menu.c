#include <PR/rdb.h>
#include <ultra64.h>

#include "console.h"
#include "controller.h"
#include "remap.h"

s32 osBbUsbInit(void);

static enum {
    NORMAL,
    HOLD_CONFIRM,
    CONFIRM,
    HOLD_BACK,
    BACK,
    HOLD_UP,
    HOLD_DOWN,
    DONE,
} mode = NORMAL;

#define MAX_SUBMENUS (10)

typedef enum { FUNCTION, SUBMENU } EntryType;

typedef struct Menu_s Menu;

typedef struct {
    EntryType type;
    char *text;
    union {
        void (*function)(void);
        const Menu *submenu;
    };
} MenuEntry;

struct Menu_s {
    size_t numEntries;
    MenuEntry entries[];
};

static unsigned int stack_ptr = 0;
static const Menu *menuStack[MAX_SUBMENUS];
static const Menu *curMenu = NULL;
static size_t selected = 0;

static void switchMenu(const Menu *newMenu) {
    curMenu = newMenu;
    selected = 0;

    console_clear();

    for (size_t i = 0; i < curMenu->numEntries; i++) {
        if (i != selected) {
            print_str(WHITE, 0, i, curMenu->entries[i].text);
        } else {
            print_str(RED, 0, i, curMenu->entries[i].text);
        }
    }
}

static struct {
    unsigned int a : 1;
    unsigned int b : 1;
    unsigned int z : 1;
    unsigned int l : 1;
    unsigned int r : 1;
    unsigned int start : 1;
    unsigned int c_up : 1;
    unsigned int c_down : 1;
    unsigned int c_left : 1;
    unsigned int c_right : 1;
    unsigned int d_up : 1;
    unsigned int d_down : 1;
    unsigned int d_left : 1;
    unsigned int d_right : 1;
    unsigned int ana_up : 1;
    unsigned int ana_down : 1;
    unsigned int ana_left : 1;
    unsigned int ana_right : 1;
} __attribute__((packed)) controllerData;

static void menuUp(void) {
    if (selected > 0) {
        print_str(WHITE, 0, selected, curMenu->entries[selected].text);
        selected--;
        print_str(RED, 0, selected, curMenu->entries[selected].text);
    }
}

static void menuDown(void) {
    if (selected < curMenu->numEntries - 1) {
        print_str(WHITE, 0, selected, curMenu->entries[selected].text);
        selected++;
        print_str(RED, 0, selected, curMenu->entries[selected].text);
    }
}

static void descend(const Menu *newMenu) {
    if (stack_ptr < MAX_SUBMENUS - 1) {
        menuStack[stack_ptr++] = curMenu;
        switchMenu(newMenu);
    } else {
        for (size_t i = 0; i < MAX_SUBMENUS - 1; i++) {
            menuStack[i] = menuStack[i + 1];
        }
        menuStack[stack_ptr] = curMenu;
        switchMenu(newMenu);
    }
}

static void ascend(void) {
    if (stack_ptr > 0) {
        stack_ptr--;
        switchMenu(menuStack[stack_ptr]);
    }
}

static void startXInput(void) {
    osBbUsbInit();
    // hook_usb(XINPUT);
    mode = DONE;
}

static void startPokken(void) {
    osBbUsbInit();
    // hook_usb(POKKEN);
    mode = DONE;
}

static void startN64(void) {
    osBbUsbInit();
    // hook_usb(N64);
    mode = DONE;
}

static void startMass(void) {
    osBbUsbInit();
    // hook_usb(MASS);
    mode = DONE;
}

static const Menu optionsMenu = {1, {{FUNCTION, "BACK", .function = ascend}}};

static const Menu mainMenu = {
    5,
    {
        {FUNCTION, "XInput controller", .function = startXInput},
        {FUNCTION, "Pokken controller", .function = startPokken},
        {FUNCTION, "N64 controller", .function = startN64},
        {FUNCTION, "Mass storage", .function = startMass},
        {SUBMENU, "Options", .submenu = &optionsMenu},
    },
};

void menu(void) {
    static char c = 'A';
    static char buf[2] = {0};

    if (mode == DONE) {
        return;
    }

    if (curMenu == NULL) {
        switchMenu(&mainMenu);
        mode = NORMAL;
    }

    parse_remap(&controllerData, sizeof(controllerData), passthrough);

    buf[0] = c;
    c++;
    if (c > '~') {
        c = ' ';
    }
    print_str(GREEN, 0, curMenu->numEntries, buf);
    switch (mode) {
        case NORMAL:
            {
                if (controllerData.a) {
                    mode = HOLD_CONFIRM;
                } else if (controllerData.b) {
                    mode = HOLD_BACK;
                } else if (controllerData.ana_up) {
                    menuUp();
                    mode = HOLD_UP;
                } else if (controllerData.ana_down) {
                    menuDown();
                    mode = HOLD_DOWN;
                }
                break;
            }
        case HOLD_CONFIRM:
            {
                if (!controllerData.a) {
                    mode = CONFIRM;
                }
                break;
            }
        case CONFIRM:
            {
                mode = NORMAL;
                switch (curMenu->entries[selected].type) {
                    case FUNCTION:
                        curMenu->entries[selected].function();
                        break;
                    case SUBMENU:
                        descend(curMenu->entries[selected].submenu);
                        break;
                }
                break;
            }
        case HOLD_BACK:
            {
                if (!controllerData.b) {
                    mode = BACK;
                }
                break;
            }
        case BACK:
            {
                mode = NORMAL;
                ascend();
                break;
            }
        case HOLD_UP:
            {
                if (!controllerData.ana_up) {
                    mode = NORMAL;
                }
                break;
            }
        case HOLD_DOWN:
            {
                if (!controllerData.ana_down) {
                    mode = NORMAL;
                }
                break;
            }
        default:
            {
                // do nothing
            }
    }
}

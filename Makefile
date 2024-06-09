TARGET ?= build/qusb.z64
ELF := $(TARGET:.z64=.elf)

DEBUG ?= 1
BBPLAYER ?= 1

PYTHON ?= python3
ELFPATCH ?= $(PYTHON) tools/elfpatch.py
CHMOD ?= chmod

IQUECRYPT ?= iquecrypt
KEY ?= 00000000000000000000000000000000
IV ?= 00000000000000000000000000000000
APP ?= 008aeae5.app

PREFIX ?= mips64-elf-

COMPILER_PATH ?= $(N64_INST)/bin
CROSS ?= $(COMPILER_PATH)/$(PREFIX)

CROSS_CC ?= $(CROSS)gcc
CROSS_AS ?= $(CROSS)as
CROSS_LD ?= $(CROSS)ld
CROSS_AR ?= $(CROSS)ar

CROSS_RANLIB  ?= $(CROSS)ranlib
CROSS_OBJCOPY ?= $(CROSS)objcopy
CROSS_OBJDUMP ?= $(CROSS)objdump
CROSS_STRIP   ?= $(CROSS)strip

#LIBGCC_DIR ?= $(dir $(shell $(CC) --print-libgcc-file-name))

SRC_DIRS := $(shell /usr/bin/find src -type d)

C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
S_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.s))
O_FILES := $(foreach f,$(C_FILES:.c=.o),build/$f) \
           $(foreach f,$(S_FILES:.s=.o),build/$f)

LIB_DIR ?= build/lib

ifeq ($(DEBUG),1)
	LIBULTRA_VERSION ?= ultra_d
	DEBUG_FLAG := -DDEBUG
else
	LIBULTRA_VERSION ?= ultra
	DEBUG_FLAG := -DNODEBUG
endif

ifeq ($(BBPLAYER),1)
	CONSOLE_FLAG := -DBBPLAYER
else
	CONSOLE_FLAG := -DN64
endif

LIBNAMES = $(LIBULTRA_VERSION) gcc

INC := -I include -I include/PR -I include/sys -I include/gcc -I src
LIBDIRS := -L $(LIB_DIR)
LIB := $(foreach lib,$(LIBNAMES),-l$(lib))
LIBS := $(foreach lib,$(LIBNAMES),$(LIB_DIR)/lib$(lib).a)
CFLAGS := $(INC) -D_MIPS_SZLONG=32 -D_LANGUAGE_C $(CONSOLE_FLAG) $(DEBUG_FLAG) -nostdinc -fno-builtin -fno-PIC -mno-abicalls -G 0 -mabi=32 -mgp32 -Wall -Wa,-Iinclude -march=vr4300 -mtune=vr4300 -ffunction-sections -fdata-sections -g -ffile-prefix-map="$(CURDIR)"= -Os -Wall -Werror -Wno-error=deprecated-declarations -fdiagnostics-color=always
ASFLAGS := $(INC) -D_MIPS_SZLONG=32 -D_LANGUAGE_ASSEMBLY $(CONSOLE_FLAG) $(DEBUG_FLAG) -nostdinc -fno-PIC -mno-abicalls -G 0 -mabi=32 -march=vr4300 -mtune=vr4300 -Wa,-Iinclude

$(shell mkdir -p build $(foreach dir,$(SRC_DIRS) lib,build/$(dir)))

.PHONY: all clean
.SECONDARY:

all: $(TARGET)

clean:
	$(RM) -r build

clobber:
	$(RM) -r $(foreach dir,$(SRC_DIRS),build/$(dir)) $(TARGET) $(ELF)

$(TARGET): $(ELF)
	$(CROSS_OBJCOPY) -O binary $< $(@:.z64=.tmp)
	dd if=$(@:.z64=.tmp) of=$@ bs=16K conv=sync status=none
	@$(RM) -r $(@:.z64=.tmp)

app: $(TARGET)
	$(IQUECRYPT) encrypt -app $(TARGET) -key $(KEY) -iv $(IV) -o $(APP)

$(LIB_DIR)/lib%: lib/lib%.a
	$(shell mkdir -p $@)
	$(CROSS_AR) x --output=$@ $<

$(LIB_DIR)/lib%.a: $(LIB_DIR)/lib%
	$(CHMOD) +w $(wildcard $</*.o)
	$(ELFPATCH) $(wildcard $</*.o)
	@$(CROSS_STRIP) -N dummy_symbol_ $(wildcard $</*.o)
	$(CROSS_AR) r $@ $(wildcard $</*.o)
	$(CROSS_RANLIB) $@

$(ELF): $(C_FILES) $(S_FILES) $(LIBS) | $(O_FILES)
	$(CROSS_LD) -T qusb.lcf -o $@ $| $(LIBDIRS) -Map $(@:.elf=.map) $(LIB) --no-warn-mismatch

build/src/%.o: src/%.s
	$(CROSS_CC) -x assembler-with-cpp $(ASFLAGS) -c $< -o $@
	@$(CROSS_OBJDUMP) -drz $@ > $(@:.o=.s)

build/src/%.o: src/%.c
	$(CROSS_CC) $(CFLAGS) -c $< -o $@
	@$(CROSS_OBJDUMP) -drz $@ > $(@:.o=.s)

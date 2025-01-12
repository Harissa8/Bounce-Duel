TC       = riscv32-MINIRISC-elf
SHELL   := /bin/sh
PATH    := $(PATH):/ens2/inf/computer_architecture/toolchain/gcc_MINIRISC/bin
export PATH

TARGET  = esw
BUILD   = build
CC      = $(TC)-gcc
CXX     = $(TC)-g++
LD      = $(TC)-g++
AR      = $(TC)-ar
SIZE    = $(TC)-size
OBJCOPY = $(TC)-objcopy
OBJDUMP = $(TC)-objdump

GCC_TOOLCHAIN_PATH = $(patsubst %/,%,$(dir $(patsubst %/,%,$(dir $(shell which $(CC))))))
SYSROOT_PATH = $(GCC_TOOLCHAIN_PATH)/riscv32-MINIRISC-elf


################################## Mini-RISC ###################################
CFLAGS += -Iminirisc
SRC    += minirisc/minirisc.c
SRC    += minirisc/minirisc_init.S

################################### xprintf ####################################
CFLAGS += -Ixprintf
SRC    += xprintf/xprintf.c

################################### FreeRTOS ###################################
CFLAGS += -IFreeRTOS/include
CFLAGS += -IFreeRTOS/portable/GCC/Mini-RISC
SRC    += FreeRTOS/tasks.c
SRC    += FreeRTOS/timers.c
SRC    += FreeRTOS/list.c
SRC    += FreeRTOS/queue.c
SRC    += FreeRTOS/croutine.c
SRC    += FreeRTOS/stream_buffer.c
SRC    += FreeRTOS/event_groups.c
SRC    += FreeRTOS/portable/MemMang/heap_3_nosuspend.c
SRC    += FreeRTOS/portable/GCC/Mini-RISC/port.c
SRC    += FreeRTOS/portable/GCC/Mini-RISC/portASM.S

################################ Support & glue ################################
CFLAGS += -Isupport
SRC    += support/syscalls.c
SRC    += support/freertos_support.c
SRC    += support/uart.c

################################################################################

LINKER_SCRIPT = minirisc/minirisc.ld

CFLAGS    += -I.
COMPILE_COMMANDS_CFLAGS := $(CFLAGS)
CFLAGS  += -W -Wall
CFLAGS  += -Og -ggdb

CFLAGS  += -march=rv32im_zicsr -mabi=ilp32

#LDFLAGS += -Wl,-verbose
LDFLAGS += -lm -Wl,-Map=./$(BUILD)/$(TARGET).map 
LDFLAGS += -Wl,-T$(LINKER_SCRIPT)

SRCC   += $(filter %.c,$(SRC))
SRCS   += $(filter %.S,$(SRC))
SRCCPP += $(filter %.cc,$(SRC))
SRCC   += $(wildcard *.c)
SRCS   += $(wildcard *.S)
SRCCPP += $(wildcard *.cc)
OBJS    = $(addprefix $(BUILD)/, $(SRCC:.c=.o) $(SRCS:.S=.o) $(SRCCPP:.cc=.o))
DEPS    = $(addprefix $(BUILD)/, $(SRCC:.c=.d))
DEPS   += $(addprefix $(BUILD)/, $(SRCS:.S=.d))
DEPS   += $(addprefix $(BUILD)/, $(SRCCPP:.cc=.d))


.PHONY: all clean size bin hex lss exec


all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).bin $(BUILD)/$(TARGET).lss

-include $(DEPS)

$(BUILD)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@ -MMD -MP -MF"$(@:%.o=%.d)"

$(BUILD)/%.o: %.cc
	@mkdir -p $(@D)
	$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF"$(@:%.o=%.d)"

$(BUILD)/%.o: %.S
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@ -MMD -MP -MF"$(@:%.o=%.d)"

$(BUILD)/$(TARGET).elf: $(OBJS) $(LINKER_SCRIPT)
	$(LD) -o $@ $(filter %.o,$^) $(CFLAGS) $(LDFLAGS)  
	@echo "────────────────────────────────────────────────────────────────────────────────"
	@$(SIZE) $@
	@echo "────────────────────────────────────────────────────────────────────────────────"

$(BUILD)/$(TARGET).hex: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

size: $(BUILD)/$(TARGET).elf
	$(SIZE) $<

$(BUILD)/$(TARGET).lss: $(BUILD)/$(TARGET).elf
	@$(OBJDUMP) -h -d $< > $@

bin: $(BUILD)/$(TARGET).bin
	@echo "done"

hex: $(BUILD)/$(TARGET).hex
	@echo "done"

lss: $(BUILD)/$(TARGET).lss
	vim $<

exec: $(BUILD)/$(TARGET).bin $(BUILD)/$(TARGET).lss
	harvey --insn=minirisc -e $<

gdb1: $(BUILD)/$(TARGET).elf
	harvey --insn=minirisc --gdb

gdb2: $(BUILD)/$(TARGET).elf
	riscv32-MINIRISC-elf-gdb --tui $<

compile_commands.json:
	@echo "[\n "$(foreach file, $(SRCC),"{\n  \"arguments\": [\n   \"clang\",\n   \"--sysroot=$(SYSROOT_PATH)\",\n   \"--gcc-toolchain=$(GCC_TOOLCHAIN_PATH)\",\n   \"--target=riscv32-unknown-elf\",\n   \"-march=rv32im\",\n   \"-mabi=ilp32\",\n   $(foreach inc,$(COMPILE_COMMANDS_CFLAGS),\"$(inc)\",\n  ) \"-c\",\n   \"-o\",\n   \"$(abspath $(addprefix $(BUILD)/, $(file:.c=.o)))\",\n   \"$(abspath $(file))\"\n  ],\n  \"directory\": \"$(abspath $(PWD))\",\n  \"file\": \"$(abspath $(file))\",\n  \"output\": \"$(abspath $(addprefix $(BUILD)/, $(file:.c=.o)))\"\n },\n")"]" > $@


clean:
	@rm -rf $(BUILD)


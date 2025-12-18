# Makefile for BLE Beacon Buzzer Control Firmware
# This is a template - adjust paths according to your Dialog SDK installation

# Project name
PROJECT_NAME = ble_beacon_buzzer_control

# Dialog SDK root directory (update this path)
SDK_ROOT ?= ~/Dialog/SDK_6.0.14

# Toolchain
TOOLCHAIN_ROOT ?= /opt/gcc-arm-none-eabi-9-2020-q2-update
PREFIX = arm-none-eabi-

# Tools
CC = $(PREFIX)gcc
AS = $(PREFIX)as
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SIZE = $(PREFIX)size

# Target
TARGET = DA14531
BUILD_DIR = build

# Source files
SOURCES = \
	src/swt_t1_beacon.c \
	src/user_barebone_beacon.c

# Include paths (adjust according to SDK structure)
INCLUDES = \
	-I./src \
	-I$(SDK_ROOT)/sdk/platform/include \
	-I$(SDK_ROOT)/sdk/platform/core_modules/include

# Defines
DEFINES = \
	-D__DA14531__ \
	-DCFG_BLE_BEACON \
	-DUSE_BLE_STACK

# Compiler flags
CFLAGS = \
	-mcpu=cortex-m0plus \
	-mthumb \
	-Os \
	-g3 \
	-Wall \
	-ffunction-sections \
	-fdata-sections \
	$(DEFINES) \
	$(INCLUDES)

# Linker flags
LDFLAGS = \
	-mcpu=cortex-m0plus \
	-mthumb \
	-Wl,--gc-sections \
	-specs=nano.specs

# Object files
OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)

# Default target
all: $(BUILD_DIR)/$(PROJECT_NAME).hex

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/src

# Compile C source files
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@echo "Compiling $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Link
$(BUILD_DIR)/$(PROJECT_NAME).elf: $(OBJECTS)
	@echo "Linking $@"
	$(CC) $(LDFLAGS) $^ -o $@
	$(SIZE) $@

# Generate hex file
$(BUILD_DIR)/$(PROJECT_NAME).hex: $(BUILD_DIR)/$(PROJECT_NAME).elf
	@echo "Creating hex file $@"
	$(OBJCOPY) -O ihex $< $@

# Generate binary file
$(BUILD_DIR)/$(PROJECT_NAME).bin: $(BUILD_DIR)/$(PROJECT_NAME).elf
	@echo "Creating binary file $@"
	$(OBJCOPY) -O binary $< $@

# Clean
clean:
	@echo "Cleaning build artifacts"
	rm -rf $(BUILD_DIR)

# Flash (using J-Link Commander)
flash: $(BUILD_DIR)/$(PROJECT_NAME).hex
	@echo "Flashing to DA14531..."
	JLinkExe -device DA14531 -if SWD -speed 4000 -autoconnect 1 -CommanderScript flash.jlink

# Generate disassembly
disasm: $(BUILD_DIR)/$(PROJECT_NAME).elf
	$(OBJDUMP) -d $< > $(BUILD_DIR)/$(PROJECT_NAME).dis

# Help
help:
	@echo "BLE Beacon Buzzer Control Firmware Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build firmware (default)"
	@echo "  clean    - Remove build artifacts"
	@echo "  flash    - Flash firmware to device"
	@echo "  disasm   - Generate disassembly listing"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "Variables:"
	@echo "  SDK_ROOT       - Dialog SDK path (default: ~/Dialog/SDK_6.0.14)"
	@echo "  TOOLCHAIN_ROOT - ARM GCC toolchain path"
	@echo ""
	@echo "Example:"
	@echo "  make SDK_ROOT=/path/to/SDK_6.0.14"
	@echo "  make clean all flash"

.PHONY: all clean flash disasm help

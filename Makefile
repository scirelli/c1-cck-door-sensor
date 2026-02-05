# STM32F405 Feather Makefile
# Board: STMicroelectronics FEATHER_F405
#
# This Makefile builds everything from source - no Arduino IDE required after
# initial toolchain/library installation.
#
# Usage:
#   make          - Build the project (builds core.a if needed)
#   make upload   - Build and upload via DFU
#   make clean    - Clean build artifacts
#   make print-vars - Debug: print configuration variables

# =============================================================================
# OPTIMIZATION #
# =============================================================================
MAKEFLAGS += -j$(shell nproc 2>/dev/null || echo 1)

# =============================================================================
# PROJECT SETTINGS
# =============================================================================
OS := $(shell uname -s)

ifeq ($(OS),Darwin)
BASE_ARDUINO    = $(HOME)/Library/Arduino15
BASE_USER_LIBS  = $(HOME)/Projects/ArduinoLibs/libraries/
else
BASE_ARDUINO    = $(HOME)/.arduino15
BASE_USER_LIBS  = $(HOME)/Arduino/libraries
endif

SERIAL_PORT ?= /dev/ttyACM0
BAUD_RATE ?= 115200

# ============================================================================
# Configuration - Adjust these paths for your system
# ============================================================================
ARDUINO_HOME    ?= $(BASE_ARDUINO)
ARDUINO_LIBS    ?= $(BASE_USER_LIBS)
STM32_VERSION   := 2.12.0
GCC_VERSION     := 14.2.1-1.1

# Project settings
TARGET          := door-main
SRC_DIR         := .
BUILD_DIR       := build

# ============================================================================
# Toolchain
# ============================================================================
TOOLCHAIN_PATH  := $(ARDUINO_HOME)/packages/STMicroelectronics/tools/xpack-arm-none-eabi-gcc/$(GCC_VERSION)/bin
CC              := $(TOOLCHAIN_PATH)/arm-none-eabi-gcc
CXX             := $(TOOLCHAIN_PATH)/arm-none-eabi-g++
OBJCOPY         := $(TOOLCHAIN_PATH)/arm-none-eabi-objcopy
SIZE            := $(TOOLCHAIN_PATH)/arm-none-eabi-size
AR              := $(TOOLCHAIN_PATH)/arm-none-eabi-ar

# ============================================================================
# Platform paths
# ============================================================================
STM32_CORE      := $(ARDUINO_HOME)/packages/STMicroelectronics/hardware/stm32/$(STM32_VERSION)
CMSIS           := $(ARDUINO_HOME)/packages/STMicroelectronics/tools/CMSIS/6.2.0/CMSIS/Core/Include
CMSIS_DSP       := $(ARDUINO_HOME)/packages/STMicroelectronics/tools/CMSIS_DSP/1.16.2
CMSIS_DEVICE    := $(STM32_CORE)/system/Drivers/CMSIS/Device/ST/STM32F4xx

# Variant
VARIANT_DIR     := $(STM32_CORE)/variants/STM32F4xx/F405RGT_F415RGT

# ============================================================================
# CPU/Architecture flags
# ============================================================================
CPU_FLAGS       := -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb

# ============================================================================
# Preprocessor defines
# ============================================================================
DEFINES := \
	-DVECT_TAB_OFFSET=0x0 \
	-DUSE_HAL_DRIVER \
	-DUSE_FULL_LL_DRIVER \
	-DNDEBUG \
	-DSTM32F4xx \
	-DARDUINO=10607 \
	-DARDUINO_FEATHER_F405 \
	-DARDUINO_ARCH_STM32 \
	-DBOARD_NAME=\"FEATHER_F405\" \
	-DVARIANT_H=\"variant_FEATHER_F405.h\" \
	-DSTM32F405xx \
	-DUSBCON \
	-DUSBD_VID=0x0483 \
	-DUSBD_PID=0x5740 \
	-DHAL_PCD_MODULE_ENABLED \
	-DUSBD_USE_CDC \
	-DHAL_UART_MODULE_ENABLED

# ============================================================================
# Include paths
# ============================================================================
INCLUDES := \
	-I$(SRC_DIR) \
	-I$(STM32_CORE)/cores/arduino \
	-I$(STM32_CORE)/cores/arduino/avr \
	-I$(STM32_CORE)/cores/arduino/stm32 \
	-I$(VARIANT_DIR) \
	-I$(CMSIS) \
	-I$(CMSIS_DSP)/Include \
	-I$(CMSIS_DSP)/PrivateInclude \
	-I$(CMSIS_DEVICE)/Include \
	-I$(CMSIS_DEVICE)/Source/Templates/gcc \
	-I$(STM32_CORE)/system/Drivers/STM32F4xx_HAL_Driver/Inc \
	-I$(STM32_CORE)/system/Drivers/STM32F4xx_HAL_Driver/Src \
	-I$(STM32_CORE)/system/STM32F4xx \
	-I$(STM32_CORE)/libraries/SrcWrapper/inc \
	-I$(STM32_CORE)/libraries/SrcWrapper/inc/LL \
	-I$(STM32_CORE)/libraries/Wire/src \
	-I$(STM32_CORE)/libraries/SPI/src \
	-I$(STM32_CORE)/libraries/USBDevice/inc \
	-I$(STM32_CORE)/libraries/VirtIO/inc \
	-I$(STM32_CORE)/system/Middlewares/ST/STM32_USB_Device_Library/Core/Inc \
	-I$(STM32_CORE)/system/Middlewares/ST/STM32_USB_Device_Library/Core/Src \
	-I$(STM32_CORE)/system/Middlewares/OpenAMP \
	-I$(STM32_CORE)/system/Middlewares/OpenAMP/open-amp/lib/include \
	-I$(STM32_CORE)/system/Middlewares/OpenAMP/libmetal/lib/include \
	-I$(STM32_CORE)/system/Middlewares/OpenAMP/virtual_driver \
	-I$(ARDUINO_LIBS)/STM32duino_STM32SD/src \
	-I$(ARDUINO_LIBS)/FatFs/src \
	-I$(ARDUINO_LIBS)/FatFs/src/drivers \
	-I$(ARDUINO_LIBS)/Adafruit_LSM6DS \
	-I$(ARDUINO_LIBS)/Adafruit_BusIO \
	-I$(ARDUINO_LIBS)/Adafruit_Unified_Sensor \
	-I$(ARDUINO_LIBS)/Adafruit_LIS3MDL

# ============================================================================
# Compiler flags
# ============================================================================
COMMON_FLAGS    := $(CPU_FLAGS) $(DEFINES) $(INCLUDES) -Os -ffunction-sections -fdata-sections

CFLAGS          := $(COMMON_FLAGS) -std=gnu11

CXXFLAGS        := $(COMMON_FLAGS) \
	-std=gnu++17 \
	-fno-threadsafe-statics \
	--param max-inline-insns-single=500 \
	-fno-rtti \
	-fno-exceptions \
	-fno-use-cxa-atexit

# ============================================================================
# Linker configuration
# ============================================================================
LDSCRIPT_VARIANT := $(VARIANT_DIR)/ldscript.ld
LDSCRIPT_SYSTEM  := $(STM32_CORE)/system/ldscript.ld

LDFLAGS := \
$(CPU_FLAGS) \
	-Os \
	--specs=nano.specs \
	-Wl,--defsym=LD_FLASH_OFFSET=0x0 \
	-Wl,--defsym=LD_MAX_SIZE=1048576 \
	-Wl,--defsym=LD_MAX_DATA_SIZE=131072 \
	-Wl,--cref \
	-Wl,--check-sections \
	-Wl,--gc-sections \
	-Wl,--entry=Reset_Handler \
	-Wl,--unresolved-symbols=report-all \
	-Wl,--warn-common \
	-Wl,--default-script=$(LDSCRIPT_VARIANT) \
	-Wl,--script=$(LDSCRIPT_SYSTEM) \
	-Wl,-Map,$(BUILD_DIR)/$(TARGET).map \
	-Wl,--no-warn-rwx-segments

LDLIBS := -lc -lm -lgcc -lstdc++

# ============================================================================
# Source files
# ============================================================================
# Your project sources
PROJECT_CPP_SRCS := $(wildcard $(SRC_DIR)/*.cpp)
PROJECT_C_SRCS   := $(wildcard $(SRC_DIR)/*.c)

# Arduino core library (precompiled)
CORE_LIB := $(ARDUINO_HOME)/packages/STMicroelectronics/hardware/stm32/$(STM32_VERSION)/cores/arduino

# Library sources - STM32SD
STM32SD_SRCS := \
	$(ARDUINO_LIBS)/STM32duino_STM32SD/src/SD.cpp \
	$(ARDUINO_LIBS)/STM32duino_STM32SD/src/Sd2Card.cpp \
	$(ARDUINO_LIBS)/STM32duino_STM32SD/src/SdFatFs.cpp \
	$(ARDUINO_LIBS)/STM32duino_STM32SD/src/bsp_sd.c

# Library sources - FatFs
FATFS_SRCS := \
	$(ARDUINO_LIBS)/FatFs/src/diskio.c \
	$(ARDUINO_LIBS)/FatFs/src/drivers/sd_diskio.c \
	$(ARDUINO_LIBS)/FatFs/src/ff.c \
	$(ARDUINO_LIBS)/FatFs/src/ff_gen_drv.c \
	$(ARDUINO_LIBS)/FatFs/src/ffsystem.c \
	$(ARDUINO_LIBS)/FatFs/src/ffunicode.c

# Library sources - Adafruit
ADAFRUIT_SRCS := \
	$(ARDUINO_LIBS)/Adafruit_LSM6DS/Adafruit_LSM6DS.cpp \
	$(ARDUINO_LIBS)/Adafruit_LSM6DS/Adafruit_LSM6DSOX.cpp \
	$(ARDUINO_LIBS)/Adafruit_LSM6DS/Adafruit_ISM330DHCX.cpp \
	$(ARDUINO_LIBS)/Adafruit_LSM6DS/Adafruit_LSM6DS3.cpp \
	$(ARDUINO_LIBS)/Adafruit_LSM6DS/Adafruit_LSM6DS33.cpp \
	$(ARDUINO_LIBS)/Adafruit_LSM6DS/Adafruit_LSM6DS3TRC.cpp \
	$(ARDUINO_LIBS)/Adafruit_LSM6DS/Adafruit_LSM6DSL.cpp \
	$(ARDUINO_LIBS)/Adafruit_LSM6DS/Adafruit_LSM6DSO32.cpp \
	$(ARDUINO_LIBS)/Adafruit_BusIO/Adafruit_BusIO_Register.cpp \
	$(ARDUINO_LIBS)/Adafruit_BusIO/Adafruit_GenericDevice.cpp \
	$(ARDUINO_LIBS)/Adafruit_BusIO/Adafruit_I2CDevice.cpp \
	$(ARDUINO_LIBS)/Adafruit_BusIO/Adafruit_SPIDevice.cpp \
	$(ARDUINO_LIBS)/Adafruit_Unified_Sensor/Adafruit_Sensor.cpp \
	$(ARDUINO_LIBS)/Adafruit_LIS3MDL/Adafruit_LIS3MDL.cpp

# Platform library sources - Wire
WIRE_SRCS := \
	$(STM32_CORE)/libraries/Wire/src/Wire.cpp \
	$(STM32_CORE)/libraries/Wire/src/utility/twi.c

# Platform library sources - SPI
SPI_SRCS := \
	$(STM32_CORE)/libraries/SPI/src/SPI.cpp \
	$(STM32_CORE)/libraries/SPI/src/utility/spi_com.c

# Platform library sources - USBDevice
USBDEVICE_SRCS := \
	$(STM32_CORE)/libraries/USBDevice/src/USBSerial.cpp \
	$(STM32_CORE)/libraries/USBDevice/src/cdc/cdc_queue.c \
	$(STM32_CORE)/libraries/USBDevice/src/cdc/usbd_cdc.c \
	$(STM32_CORE)/libraries/USBDevice/src/cdc/usbd_cdc_if.c \
	$(STM32_CORE)/libraries/USBDevice/src/hid/usbd_hid_composite.c \
	$(STM32_CORE)/libraries/USBDevice/src/hid/usbd_hid_composite_if.c \
	$(STM32_CORE)/libraries/USBDevice/src/usb_device_core.c \
	$(STM32_CORE)/libraries/USBDevice/src/usb_device_ctlreq.c \
	$(STM32_CORE)/libraries/USBDevice/src/usb_device_ioreq.c \
	$(STM32_CORE)/libraries/USBDevice/src/usbd_conf.c \
	$(STM32_CORE)/libraries/USBDevice/src/usbd_desc.c \
	$(STM32_CORE)/libraries/USBDevice/src/usbd_ep_conf.c \
	$(STM32_CORE)/libraries/USBDevice/src/usbd_if.c

# Variant sources
VARIANT_SRCS := \
	$(VARIANT_DIR)/variant_FEATHER_F405.cpp \
	$(VARIANT_DIR)/PeripheralPins_FEATHER_F405.c

# ============================================================================
# SrcWrapper library sources (HAL/LL drivers) - for building core.a
# ============================================================================
SRCWRAPPER_DIR := $(STM32_CORE)/libraries/SrcWrapper/src

# HAL drivers
SRCWRAPPER_HAL_SRCS := $(wildcard $(SRCWRAPPER_DIR)/HAL/*.c)

# LL drivers
SRCWRAPPER_LL_SRCS := $(wildcard $(SRCWRAPPER_DIR)/LL/*.c)

# STM32 support files
SRCWRAPPER_STM32_SRCS := \
	$(SRCWRAPPER_DIR)/stm32/PortNames.c \
	$(SRCWRAPPER_DIR)/stm32/bootloader.c \
	$(SRCWRAPPER_DIR)/stm32/clock.c \
	$(SRCWRAPPER_DIR)/stm32/core_callback.c \
	$(SRCWRAPPER_DIR)/stm32/dwt.c \
	$(SRCWRAPPER_DIR)/stm32/hw_config.c \
	$(SRCWRAPPER_DIR)/stm32/otp.c \
	$(SRCWRAPPER_DIR)/stm32/pinmap.c \
	$(SRCWRAPPER_DIR)/stm32/stm32_def.c \
	$(SRCWRAPPER_DIR)/stm32/system_stm32yyxx.c \
	$(SRCWRAPPER_DIR)/stm32/timer.c \
	$(SRCWRAPPER_DIR)/stm32/uart.c

SRCWRAPPER_STM32_CPP_SRCS := \
	$(SRCWRAPPER_DIR)/stm32/analog.cpp \
	$(SRCWRAPPER_DIR)/stm32/interrupt.cpp

# Other SrcWrapper files
SRCWRAPPER_OTHER_SRCS := \
	$(SRCWRAPPER_DIR)/syscalls.c

SRCWRAPPER_OTHER_CPP_SRCS := \
	$(SRCWRAPPER_DIR)/HardwareTimer.cpp \
	$(SRCWRAPPER_DIR)/new.cpp

# Startup assembly file (contains Reset_Handler and interrupt vector table)
ASM_SRCS := $(STM32_CORE)/system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f405xx.s

# All SrcWrapper sources
SRCWRAPPER_C_SRCS := $(SRCWRAPPER_HAL_SRCS) $(SRCWRAPPER_LL_SRCS) $(SRCWRAPPER_STM32_SRCS) $(SRCWRAPPER_OTHER_SRCS)
SRCWRAPPER_CPP_SRCS := $(SRCWRAPPER_STM32_CPP_SRCS) $(SRCWRAPPER_OTHER_CPP_SRCS)

# ============================================================================
# Arduino core sources - for building core.a
# ============================================================================
ARDUINO_CORE_DIR := $(STM32_CORE)/cores/arduino

ARDUINO_CORE_CPP_SRCS := \
	$(ARDUINO_CORE_DIR)/abi.cpp \
	$(ARDUINO_CORE_DIR)/board.cpp \
	$(ARDUINO_CORE_DIR)/HardwareSerial.cpp \
	$(ARDUINO_CORE_DIR)/hooks.cpp \
	$(ARDUINO_CORE_DIR)/IPAddress.cpp \
	$(ARDUINO_CORE_DIR)/main.cpp \
	$(ARDUINO_CORE_DIR)/pins_arduino.cpp \
	$(ARDUINO_CORE_DIR)/Print.cpp \
	$(ARDUINO_CORE_DIR)/RingBuffer.cpp \
	$(ARDUINO_CORE_DIR)/Stream.cpp \
	$(ARDUINO_CORE_DIR)/Tone.cpp \
	$(ARDUINO_CORE_DIR)/WMath.cpp \
	$(ARDUINO_CORE_DIR)/WSerial.cpp \
	$(ARDUINO_CORE_DIR)/WString.cpp

ARDUINO_CORE_C_SRCS := \
	$(ARDUINO_CORE_DIR)/itoa.c \
	$(ARDUINO_CORE_DIR)/wiring_analog.c \
	$(ARDUINO_CORE_DIR)/wiring_digital.c \
	$(ARDUINO_CORE_DIR)/wiring_shift.c \
	$(ARDUINO_CORE_DIR)/wiring_time.c

# Note: wiring_pulse is actually a .cpp file
ARDUINO_CORE_CPP_SRCS += $(ARDUINO_CORE_DIR)/wiring_pulse.cpp

# Combine all library sources (user libraries)
# Note: VARIANT_SRCS are compiled into core.a, not as separate library objects
LIB_CPP_SRCS := $(filter %.cpp,$(STM32SD_SRCS) $(ADAFRUIT_SRCS) $(WIRE_SRCS) $(SPI_SRCS) $(USBDEVICE_SRCS))
LIB_C_SRCS   := $(filter %.c,$(STM32SD_SRCS) $(FATFS_SRCS) $(WIRE_SRCS) $(SPI_SRCS) $(USBDEVICE_SRCS))

# ============================================================================
# Object files
# ============================================================================
PROJECT_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(PROJECT_CPP_SRCS))
PROJECT_OBJS += $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(PROJECT_C_SRCS))

# Library objects use basename - ensure no duplicate filenames across libraries
LIB_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/libs/%.o,$(notdir $(LIB_CPP_SRCS)))
LIB_OBJS += $(patsubst %.c,$(BUILD_DIR)/libs/%.o,$(notdir $(LIB_C_SRCS)))

ALL_OBJS := $(PROJECT_OBJS) $(LIB_OBJS)

# ============================================================================
# Core library (core.a) - built from source
# ============================================================================
CORE_A := $(BUILD_DIR)/core.a

# Core object files
CORE_OBJS := $(patsubst %.c,$(BUILD_DIR)/core/%.o,$(notdir $(SRCWRAPPER_C_SRCS)))
CORE_OBJS += $(patsubst %.cpp,$(BUILD_DIR)/core/%.o,$(notdir $(SRCWRAPPER_CPP_SRCS)))
CORE_OBJS += $(patsubst %.s,$(BUILD_DIR)/core/%.o,$(notdir $(ASM_SRCS)))
CORE_OBJS += $(patsubst %.c,$(BUILD_DIR)/core/%.o,$(notdir $(ARDUINO_CORE_C_SRCS)))
CORE_OBJS += $(patsubst %.cpp,$(BUILD_DIR)/core/%.o,$(notdir $(ARDUINO_CORE_CPP_SRCS)))
CORE_OBJS += $(patsubst %.c,$(BUILD_DIR)/core/%.o,$(notdir $(filter %.c,$(VARIANT_SRCS))))
CORE_OBJS += $(patsubst %.cpp,$(BUILD_DIR)/core/%.o,$(notdir $(filter %.cpp,$(VARIANT_SRCS))))

# ============================================================================
# VPATH for library and core sources
# ============================================================================
VPATH := $(sort $(dir $(LIB_CPP_SRCS) $(LIB_C_SRCS) $(SRCWRAPPER_C_SRCS) $(SRCWRAPPER_CPP_SRCS) $(ASM_SRCS) $(ARDUINO_CORE_C_SRCS) $(ARDUINO_CORE_CPP_SRCS) $(VARIANT_SRCS)))

# ============================================================================
# Assembly flags
# ============================================================================
ASFLAGS := $(CPU_FLAGS) $(DEFINES) $(INCLUDES) -x assembler-with-cpp

# ============================================================================
# Targets
# ============================================================================
.PHONY: all clean upload size monitor send term core

all: $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).hex size

# Build core.a from source
core: $(CORE_A)

$(CORE_A): $(CORE_OBJS) | $(BUILD_DIR)
	@echo "Creating core library $@..."
	$(AR) rcs $@ $^

$(BUILD_DIR)/$(TARGET).elf: $(ALL_OBJS) $(CORE_A) | $(BUILD_DIR)
	@echo "Linking $@..."
	$(CC) $(LDFLAGS) -L$(BUILD_DIR) -Wl,--start-group $^ $(LDLIBS) -Wl,--end-group -o $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	@echo "Creating $@..."
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	@echo "Creating $@..."
	$(OBJCOPY) -O ihex $< $@

# Compile project C++ files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile project C files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Compile library C++ files
$(BUILD_DIR)/libs/%.o: %.cpp | $(BUILD_DIR)/libs
	@echo "Compiling library $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile library C files
$(BUILD_DIR)/libs/%.o: %.c | $(BUILD_DIR)/libs
	@echo "Compiling library $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Compile core C++ files
$(BUILD_DIR)/core/%.o: %.cpp | $(BUILD_DIR)/core
	@echo "Compiling core $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile core C files
$(BUILD_DIR)/core/%.o: %.c | $(BUILD_DIR)/core
	@echo "Compiling core $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Compile core assembly files (.s) - explicit rule for startup file
$(BUILD_DIR)/core/startup_stm32f405xx.o: $(ASM_SRCS) | $(BUILD_DIR)/core
	@echo "Assembling core $<..."
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/libs:
	mkdir -p $(BUILD_DIR)/libs

$(BUILD_DIR)/core:
	mkdir -p $(BUILD_DIR)/core

size: $(BUILD_DIR)/$(TARGET).elf
	@echo ""
	@echo "Size:"
	$(SIZE) -A $<

upload: $(BUILD_DIR)/$(TARGET).bin
	@echo "Uploading via DFU..."
	dfu-util --alt 0 --dfuse-address 0x08000000:leave --download $<

clean:
	rm -rf $(BUILD_DIR)

monitor:
	@echo "Opening serial monitor on $(SERIAL_PORT) at $(BAUD_RATE) baud..."
	@stty -F $(SERIAL_PORT) $(BAUD_RATE) raw -clocal -echo
	@cat $(SERIAL_PORT)

# Send: Send a message to the serial port
# Usage: make send MSG="Hello World"
send:
	@if [ -z "$(MSG)" ]; then \
		echo "Usage: make send MSG=\"Your Message\""; \
	else \
		echo "Sending '$(MSG)' to $(SERIAL_PORT)..."; \
		stty -F $(SERIAL_PORT) $(BAUD_RATE) raw -clocal -echo; \
		echo "$(MSG)" > $(SERIAL_PORT); \
	fi

# Term: Interactive session (requires 'screen' installed)
term:
	@echo "Opening interactive terminal on $(SERIAL_PORT)..."
	@echo "Press Ctrl+A then K to exit."
	@screen $(SERIAL_PORT) $(BAUD_RATE)

# ============================================================================
# Debug helpers
# ============================================================================
.PHONY: print-vars
print-vars:
	@echo "TOOLCHAIN_PATH: $(TOOLCHAIN_PATH)"
	@echo "STM32_CORE: $(STM32_CORE)"
	@echo "CORE_A: $(CORE_A)"
	@echo "PROJECT_OBJS: $(PROJECT_OBJS)"
	@echo "LIB_OBJS: $(LIB_OBJS)"
	@echo "CORE_OBJS count: $(words $(CORE_OBJS))"

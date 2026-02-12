# =============================================================================
# Project Dependencies Configuration
# =============================================================================
# This file defines project-specific dependencies that are cloned from GitHub.
# The main Makefile includes this file and auto-discovers sources.
#
# Format: name::repo_url::src_subdir
#   - name: Directory name in $(USER_LIBS)
#   - repo_url: GitHub repository URL
#   - src_subdir: Subdirectory containing source files (empty = root)
#
# Example:
#   MyLib::https://github.com/user/MyLib.git::src
#   SimpleLib::https://github.com/user/SimpleLib.git::
#
# =============================================================================

# Project name (output binary will be $(TARGET).bin)
TARGET := door-main

# =============================================================================
# Library Dependencies
# =============================================================================
# Each entry specifies: name::git_url::source_subdir
# The source_subdir is where .c/.cpp files are located (empty = library root)

DEP_REPOS := \
	Adafruit_LIS3MDL::https://github.com/adafruit/Adafruit_LIS3MDL.git:: \
	Adafruit_LSM6DS::https://github.com/adafruit/Adafruit_LSM6DS.git:: \
	Adafruit_BusIO::https://github.com/adafruit/Adafruit_BusIO.git:: \
	Adafruit_Unified_Sensor::https://github.com/adafruit/Adafruit_Sensor.git:: \
	Adafruit_NeoPixel::https://github.com/adafruit/Adafruit_NeoPixel.git:: \
	Adafruit_GFX::https://github.com/adafruit/Adafruit-GFX-Library.git:: \
	Adafruit_SH110x::https://github.com/adafruit/Adafruit_SH110x.git:: \
	STM32duino_STM32SD::https://github.com/stm32duino/STM32SD.git::src \
	FatFs::https://github.com/stm32duino/FatFs.git::src

# =============================================================================
# Additional Source Files (optional)
# =============================================================================
# If auto-discovery doesn't find all needed files, or you need files from
# subdirectories, list them explicitly here.
#
# DEP_EXTRA_C_SRCS: Additional .c files
# DEP_EXTRA_CPP_SRCS: Additional .cpp files

DEP_EXTRA_C_SRCS := \
	$(USER_LIBS)/FatFs/src/drivers/sd_diskio.c

DEP_EXTRA_CPP_SRCS :=

# =============================================================================
# Excluded Files (optional)
# =============================================================================
# Files to exclude from auto-discovery (e.g., examples, tests)

DEP_EXCLUDE_PATTERNS := \
	%/examples/% \
	%/test/% \
	%/extras/%

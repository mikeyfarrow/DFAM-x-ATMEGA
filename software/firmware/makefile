# Copyright 2024 Mikey Farrow
#
# Author: Mikey Farrow (michaelkfarrow@gmail.com)
# 
# inspired by Emilie Gillet's AVR midi bootloader
# (see https://github.com/pichenettes/avr-midi-bootloader)

MCU_NAME       = 328
MCU            = atmega$(MCU_NAME)p
DMCU           = m$(MCU_NAME)p
MCU_DEFINE     = ATMEGA$(MCU_NAME)P
F_CPU          = 16000000

PROGRAMMER     = avrispmkII
LOCK           = ff
LFUSE          = ff
HFUSE          = d8
EFUSE          = fd

TARGET		   = midifam
BASE_DIR	   = firmware/
BUILD_ROOT     = build/
BUILD_DIR      = $(BUILD_ROOT)$(TARGET)/
TARGET_BIN     = $(BUILD_DIR)$(TARGET).bin
TARGET_HEX     = $(BUILD_DIR)$(TARGET).hex
TARGET_MID     = $(BUILD_DIR)$(TARGET).mid

# Compiler flags
CPPFLAGS  	   = -mmcu=$(MCU) -I. \
					-g -Os -w -Wall \
					-DF_CPU=$(F_CPU) \
					-fdata-sections \
					-ffunction-sections \
					-fshort-enums \
					-fno-move-loop-invariants \
					$(EXTRA_DEFINES) \
					$(MMC_CONFIG) \
					-D$(MCU_DEFINE) \
					-mcall-prologues
CXXFLAGS      = -fno-exceptions
ASFLAGS       = -mmcu=$(MCU) -I. -x assembler-with-cpp
LDFLAGS       = -mmcu=$(MCU) -lm -Os -Wl,--gc-sections$(EXTRA_LD_FLAGS)

PYTHON	       = /usr/local/Python-2.5/python
TOOLCHAIN_PATH = /usr/local/avr8-gnu-toolchain/bin/
AVRDUDE 	   = avrdude -c $(PRG) -p $(MCU)
CC             = $(TOOLCHAIN_PATH)avr-gcc
CXX            = $(TOOLCHAIN_PATH)avr-g++
OBJCOPY        = $(TOOLCHAIN_PATH)avr-objcopy
OBJDUMP        = $(TOOLCHAIN_PATH)avr-objdump
AR             = $(TOOLCHAIN_PATH)avr-ar
SIZE           = $(TOOLCHAIN_PATH)avr-size --format=avr --mcu=$(MCU)
NM             = $(TOOLCHAIN_PATH)avr-nm

AVRDUDE_CONF      = /etc/avrdude.conf
AVRDUDE_COM_OPTS  = -V -p $(DMCU)
AVRDUDE_COM_OPTS += -C $(AVRDUDE_CONF)
AVRDUDE_ISP_OPTS  = -c $(PROGRAMMER) -P usb

CPP_FILES = main.cpp CvOutput.cpp GPIO.cpp MidiController.cpp SerialMidiTransport.cpp 
SOURCES = $(addprefix $(BASE_DIR), $(CPP_FILES))
OBJECTS = $(SOURCES:.cpp=.o)

all:    $(BUILD_DIR) $(TARGET_MID)

$(BUILD_DIR):
		mkdir -p $(BUILD_DIR)

$(TARGET_BIN): $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGET_HEX): $(TARGET_BIN)
	$(OBJCOPY) -O ihex -R .eeprom $(TARGET_BIN) $(TARGET_HEX)

$(TARGET_MID): $(TARGET_HEX)
	$(PYTHON) tools/hex2sysex/hex2sysex.py -p 64 -d 250 -o $(TARGET_MID) $(TARGET_HEX)

upload: $(TARGET_HEX)
	$(AVRDUDE) $(AVRDUDE_COM_OPTS) $(AVRDUDE_ISP_OPTS) \
		-U flash:w:$(TARGET_HEX):i -U lock:w:0x$(LOCK):m

.PHONY: clean 

clean:
	rm -rf $(BUILD_DIR)


# # parameters (change this stuff accordingly)
# # project name
# PRJ = midifam
# # avr mcu
# MCU = atmega328p
# # mcu clock frequency
# CLK = 16000000
# # avr programmer (and port if necessary)
# # e.g. PRG = usbtiny -or- PRG = arduino -P /dev/tty.usbmodem411
# PRG = avrisp2
# # fuse values for avr: low, high, and extended
# # these values are from an Arduino Uno (ATMega328P)
# # see http://www.engbedded.com/fusecalc/ for other MCUs and options
# LFU = 0xFF
# HFU = 0xD8
# EFU = 0xFD
# # program source files (not including external libraries)
# SRC = ./
# # where to look for external libraries (consisting of .c/.cpp files and .h files)
# # e.g. EXT = ../../EyeToSee ../../YouSART
# EXT = lib/

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


# executables
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

$(TARGET_HEX): all

all: build

build: $(TARGET_HEX)
	mkdir -p $(BUILD_DIR) 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $(TARGET_BIN) $(BASE_DIR)main.cpp $(BASE_DIR)CvOutput.cpp $(BASE_DIR)GPIO.cpp $(BASE_DIR)MidiController.cpp $(BASE_DIR)SerialMidiTransport.cpp 
	$(OBJCOPY) -O ihex -R .eeprom $(TARGET_BIN) $(TARGET_HEX)

upload: all
	$(AVRDUDE) $(AVRDUDE_COM_OPTS) $(AVRDUDE_ISP_OPTS) \
		-U flash:w:$(TARGET_HEX):i -U lock:w:0x$(LOCK):m

sysex:
	$(PYTHON) tools/hex2sysex/hex2sysex.py -p 64 -d 250 -o $(TARGET_MID) $(TARGET_HEX)

clean:
	rm -rf $(BUILD_DIR)

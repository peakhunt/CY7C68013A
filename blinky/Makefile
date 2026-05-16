# Path to your fx2lib installation
FX2LIB_DIR = ../fx2lib

# Compiler and Flags
CC = sdcc
CFLAGS = -mmcs51 --std-sdcc99 -I$(FX2LIB_DIR)/include
LIBS = -L$(FX2LIB_DIR)/lib -lfx2

TARGET=blinky

# Get the lsusb info
USB_INFO := $(shell lsusb | grep "CY7C68013")
# Extract IDs
BUSID := $(word 2, $(USB_INFO))
DEVID := $(subst :,,$(word 4, $(USB_INFO)))

DEVICE_PATH := /dev/bus/usb/$(BUSID)/$(DEVID)

LOADER := ../Vendor_Ax/Vend_Ax_Eeprom.hex

VID = 0xAAAA
PID = 0xBBBB

LOADER := ../Vendor_Ax/Vend_Ax_Eeprom.hex

# Targets
all: $(TARGET).ihx

check_device:
ifeq ($(BUSID),)
	@echo "Error: Device not detected. Check EEPROM/Jumper."
	@exit 1
else
	@echo "Success: Found FX2 on Bus $(BUSID), Device $(DEVID)"
endif

$(TARGET).ihx: $(TARGET).c
	$(CC) $(CFLAGS) $(TARGET).c $(LIBS)

# Upload to RAM for testing
run: check_device $(TARGET).ihx
	cycfx2prog -d=$(BUSID).$(DEVID) prg:$(TARGET).ihx run

program: check_device
	fxload -v -D $(DEVICE_PATH) -I $(TARGET).ihx -c 0xc2 -s $(LOADER) -t fx2lp

# Clean build artifacts
clean:
	rm -f *.ihx *.rel *.asm *.lst *.sym *.mem *.rst *.map *.lk


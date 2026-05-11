# Copyright (C) 2010 Ubixum, Inc. 
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA



# change location of fx2libdir if needed
SOURCES=      fw.c          \
              device.c      \
              tm1638.c

A51_SOURCES=dscr.a51

BASENAME=hid_tm1638


# Get the lsusb info
USB_INFO := $(shell lsusb | grep "CY7C68013")
# Extract IDs
BUSID := $(word 2, $(USB_INFO))
DEVID := $(subst :,,$(word 4, $(USB_INFO)))
LOADER := ../Vendor_Ax/Vend_Ax_Eeprom.hex
DEVICE_PATH := /dev/bus/usb/$(BUSID)/$(DEVID)

#
# FX2LP has no uart pins exposed
# wonderfun decision by chip manufacturer
#
#SDCCFLAGS := -DDEBUG_FIRMWARE
# Allow code up to 10KB
CODE_SIZE = --code-size 0x2800

# Start variables at 10KB (0x2800)
XRAM_LOC  = --xram-loc 0x2800

# Allow 4KB for variables (plenty for a 2KB buffer)
XRAM_SIZE = --xram-size 0x1000

FX2LIBDIR=../fx2lib
include $(FX2LIBDIR)/lib/fx2.mk

check_device:
ifeq ($(BUSID),)
	@echo "Error: Device not detected. Check EEPROM/Jumper."
	@exit 1
else
	@echo "Success: Found FX2 on Bus $(BUSID), Device $(DEVID)"
endif

program: check_device
	fxload -v -D $(DEVICE_PATH) -I $(BUILDDIR)/$(BASENAME).ihx -c 0xc2 -s $(LOADER) -t fx2lp

run: check_device $(BUILDDIR)/$(BASENAME).ihx
	cycfx2prog -d=$(BUSID).$(DEVID) prg:$(BUILDDIR)/$(BASENAME).ihx run

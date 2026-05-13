/**
 * Copyright (C) 2009 Ubixum, Inc. 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **/

#include <fx2macros.h>
#include <fx2regs.h>
#include <fx2ints.h>
#include <delay.h>
#include <stdint.h>

#define T1_INTERVAL     200

#define SYNCDELAY() SYNCDELAY4

#ifdef DEBUG_FIRMWARE
#include <stdio.h>
#else
#define printf(...)
#endif

volatile WORD t1 = 0;
volatile WORD ms_count = 0;
volatile BOOL d2_led = FALSE;
volatile uint32_t uptime_sec;

uint32_t
get_uptime(void)
{
    uint32_t temp;

    ET0 = 0;           // Mask Timer 0 only
    temp = uptime_sec; // Fast 32-bit copy
    ET0 = 1;           // Unmask immediately
                       //
    return temp;
}

void
timer0_isr(void) __interrupt (TF0_ISR)
{
  ms_count++;
  if(ms_count >= 1000)
  {
    uptime_sec++;
    ms_count = 0;
    d2_led = !d2_led;
  }

  TH0 = 0xF0;
  TL0 = 0x60;

  t1++;

  if(t1 > T1_INTERVAL)
  {
    t1 = 0;
    IOA ^= 0x01;
  }
}

BOOL
handle_get_descriptor(void)
{
  return FALSE;
}


// change to support as many interfaces as you need
//volatile xdata BYTE interface=0;
//volatile xdata BYTE alt=0; // alt interface

// set *alt_ifc to the current alt interface for ifc
BOOL
handle_get_interface(BYTE ifc, BYTE* alt_ifc)
{
  (void)ifc;
  (void)alt_ifc;
  // *alt_ifc=alt;
  return TRUE;
}

// return TRUE if you set the interface requested
// NOTE this function should reconfigure and reset the endpoints
// according to the interface descriptors you provided.
BOOL
handle_set_interface(BYTE ifc,BYTE alt_ifc)
{  
  (void)ifc;
  (void)alt_ifc;
  printf ( "Set Interface.\n" );
  //interface=ifc;
  //alt=alt_ifc;
  return TRUE;
}

// handle getting and setting the configuration
// 1 is the default.  If you support more than one config
// keep track of the config number and return the correct number
// config numbers are set int the dscr file.
//volatile BYTE config=1;
BYTE
handle_get_configuration(void)
{ 
  return 1;
}

// NOTE changing config requires the device to reset all the endpoints
BOOL
handle_set_configuration(BYTE cfg)
{ 
  (void)cfg;
  printf ( "Set Configuration.\n" );
  //config=cfg;
  return TRUE;
}


//******************* VENDOR COMMAND HANDLERS **************************
BOOL
handle_vendorcommand(BYTE cmd)
{
  (void)cmd;
  return FALSE;
}

//********************  INIT ***********************
void
main_init(void)
{
  REVCTL=3;
  SYNCDELAY();
  SETIF48MHZ();

  // Lock EP2 as a valid Bulk-OUT endpoint
  // 0xA2 = Endpoint Active, OUT direction, Bulk type, 512 bytes, Quad-buffered
  EP2CFG = 0xA2; 
  SYNCDELAY();                    

  // Turn on AUTOOUT mode for the 8-bit bus
  // 0x10 = AUTOOUT enabled (bypasses 8052 CPU), 8-bit data bus width (WORDWIDE=0)
  EP2FIFOCFG = 0x10;            
  SYNCDELAY();

  // Bind FLAGA strictly to EP2 Empty status
  // 0x00 = FLAGA drops LOW when EP2 is completely empty, snaps HIGH when data arrives
  PINFLAGSAB = 0x00; 
  SYNCDELAY();

  // Invert FLAGA polarity to make it Active High 
  // (Bit 0 = 1 makes FLAGA High on Data Ready, Low on Empty)
  FIFOPINPOLAR = 0x01; 
  SYNCDELAY();

  //
  // setup led and timer for blinking
  // 
  OEA = 0x03;
  TMOD = 0x01; // Timer 0, Mode 1 (16-bit)

  // Set 1ms reload value (0xf060)
  TH0 = 0xf0;
  TL0 = 0x60;
  ENABLE_TIMER0();
  TR0 = 1; // Start Timer 0
           // interrupts are enabled later in main
}

void
main_loop(void)
{
  if(d2_led)
  {
    IOA &= ~0x02;
  }
  else
  {
    IOA |= 0x02;
  }
}

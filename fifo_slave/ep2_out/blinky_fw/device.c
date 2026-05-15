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
#include <eputils.h>
#include <delay.h>
#include <stdint.h>

#define T1_INTERVAL     200

#define SYNCDELAY() SYNCDELAY4

#ifdef DEBUG_FIRMWARE
#include <stdio.h>
#else
#define printf(...)
#endif

WORD t1 = 0;
WORD ms_count = 0;

void
timer0_isr(void) __interrupt (TF0_ISR)
{
  ms_count++;
  if(ms_count >= 1000)
  {
    ms_count = 0;
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
  if(ifc == 0)
  {
     *alt_ifc=ifc;
     return TRUE;
  }
  return FALSE;
}

// return TRUE if you set the interface requested
// NOTE this function should reconfigure and reset the endpoints
// according to the interface descriptors you provided.
BOOL
handle_set_interface(BYTE ifc,BYTE alt_ifc)
{  
	return ifc == 0 && alt_ifc == 0;
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
void main_init(void)
{
  /////////////////////////////////////////////////////////////////////////
  //
  // XXX
  // On my blue chinese FX2LP board
  // RDY0/RDY1 pins are mislabled.
  // RDY0 pin is labled as RDY1 and vice versa.
  // At least that was my case.
  // Test your board with multimeter if you don't wanna waste
  // several days wondering why
  //
  // CTRL0 : flag
  // RDY0(RDY1 pin on my chinese board) : slrd
  // PA02   : slor
  // PA0[4|5]: FIFOADDR[0|1]
  // PB0-7 : data0-7
  // IFCLK : IFCLK
  // 
  // hope I remembered all these correctly.
  //
  /////////////////////////////////////////////////////////////////////////
  REVCTL = 0x03;
  SYNCDELAY();

  // Force the layout pins to Slave FIFO Mode and drive the clock out
  IFCONFIG = 0xe3; 
  SYNCDELAY();

  PINFLAGSAB = 0x08; // FLAGA = EP2 EF
  SYNCDELAY();

  PORTACFG |= 0x80;    // flagd. no nCS

  EP1INCFG &= ~bmVALID;
	SYNCDELAY();

	EP1OUTCFG &= ~bmVALID;
	SYNCDELAY();

  EP2CFG = 0xa2;    // EP2 OUT, Bulk, 512, 2x
  SYNCDELAY();                    

  EP4CFG = 0x02;
	SYNCDELAY();

  EP6CFG = 0x02;
	SYNCDELAY();

  EP8CFG = 0x02;
	SYNCDELAY();

  // reset fifo
  FIFORESET = 0x80; // Activate NAK-ALL to mask the USB bus
  SYNCDELAY();
  FIFORESET = 0x02; // Force hardware reset on FIFO 2
  SYNCDELAY();
  FIFORESET = 0x00; // Release NAK-ALL
  SYNCDELAY();

  // arm both EP2 buffers to prim the pump
  OUTPKTEND = 0x82;
  SYNCDELAY();
  OUTPKTEND = 0x82;
  SYNCDELAY();

  // Bind and lock pin polarities
  FIFOPINPOLAR = 0x00;
  SYNCDELAY();

  // 
  //EP2FIFOCFG = 0x00;
  //SYNCDELAY();

  EP2FIFOCFG = 0x10; // EP2 AUTOOUT=1, AUTOIN=0, ZEROLEN=0, WORDWIDE=0
  SYNCDELAY();

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
  // to see fifo status on LED
  if(EP2FIFOFLGS & 0x02)
  {
    // fifo empty flag
    IOA |= 0x02;
  }
  else
  {
    IOA &= ~0x02;
  }
}

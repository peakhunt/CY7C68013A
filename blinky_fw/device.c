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

#include "usb_cdc.h"
#include "shell.h"

#define T1_INTERVAL     200

#define SYNCDELAY() SYNCDELAY4

#ifdef DEBUG_FIRMWARE
#include <stdio.h>
#else
#define printf(...)
#endif

#define TX_BUF_SIZE                 512
#define CDC_SET_LINE_CODING         0x20
#define CDC_GET_LINE_CODING         0x21
#define CDC_SET_CONTROL_LINE_STATE  0x22

////////////////////////////////////////////////////////////////////////////////
//
// USB CDC Related
//
////////////////////////////////////////////////////////////////////////////////
static void
cdc_handle_rx(void)
{
  uint16_t count;
  uint16_t i;

  if(!(EP2468STAT & bmEP2EMPTY))
  {
    count = MAKEWORD(EP2BCH,EP2BCL);

    for(i = 0; i < count; i++)
    {
      uint8_t rc = EP2FIFOBUF[i];
      shell_handle_rx(rc);
    }

    // re-arm
    EP2BCL = 0x80;
    SYNCDELAY();
  }
}

void
cdc_print_string(const char* str)
{
  // not a good implementation
  // but here we go.
  // shell max line is 256, which should be enough
  // for 512 bytes fifo buffer
  // and fifo is all or nothing.
  // I'll stick to this until a better idea comes
  //
  uint16_t count = 0;

  //
  // FIXME. USB error case?
  //
  while(!(EP2468STAT & bmEP6EMPTY))
    ;

  while(*str != NULL)
  {
    EP6FIFOBUF[count++] = *str;
    str++;
  }

  EP6BCH = MSB(count);
  SYNCDELAY();
  EP6BCL = LSB(count); // Ship the packet
}

///
// Dummy storage for the 7-byte line coding (Baud, Stop bits, Parity, Data bits)
// Default: 9600 baud, 1 stop, no parity, 8 data bits
__xdata uint8_t LineCoding[7] = {0x80, 0x25, 0x00, 0x00, 0x00, 0x00, 0x08};
static BOOL
handle_usb_cdc_setup_data_cmd(BYTE cmd)
{
  switch(cmd)
  {
  case CDC_SET_LINE_CODING:
    // Host is sending 7 bytes. We must tell EP0 to accept them.
    EP0BCH = 0;
    EP0BCL = 7;
    SYNCDELAY();
    while (EP0CS & bmEPBUSY); // Wait for the 7 bytes to arrive in EP0BUF
                               // If you actually had a UART, you'd parse EP0BUF here. 
                               // For now, we just ACK.
    
    for(int i = 0; i < 7; i++)
    {
      LineCoding[i] = EP0BUF[i];
    }
    return TRUE;

  case CDC_GET_LINE_CODING:
    // Host wants to know the "baud rate". Send our 7-byte dummy.
    SUDPTRH = MSB(&LineCoding);
    SUDPTRL = LSB(&LineCoding);
    // SUDPTRL write automatically handles the transfer
    return TRUE;

  case CDC_SET_CONTROL_LINE_STATE:
    // Host is toggling DTR/RTS. We don't have pins, so just say OK.
    return TRUE;
  }
  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
//
// end of USB CDC Related
//
////////////////////////////////////////////////////////////////////////////////

/*
 let's implement USB CDC here
 USB CDC Requirements:
  interface0 - communication class
    EP01    for control
    EP1IN   for interrupt

  interface1 - data class
    EP2     for Bulk Data Out
    EP6     for Bulk Data In
*/

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
  return handle_usb_cdc_setup_data_cmd(cmd);
}

//********************  INIT ***********************
void
main_init(void)
{
  REVCTL=0;
  SYNCDELAY();
  SETIF48MHZ();

  EP4CFG &= ~bmVALID;
  SYNCDELAY();
  EP8CFG &= ~bmVALID;
  SYNCDELAY();

  // Enable EP1 IN for the CDC Interrupt/Status pipe
  // Valid=1, Type=01 (Interrupt)
  EP1INCFG = 0xB0;    // valid, INT endpoint
  SYNCDELAY();
  EP1OUTCFG &= ~bmVALID; 
  SYNCDELAY();

  EP2CFG = 0xA2;      // valid, Bulk, OUT, 512 bytes, double buffered
  SYNCDELAY();
  EP6CFG = 0xE2;      // valid, Bulk, In,  512 bytes, double buffered
  SYNCDELAY();

  FIFORESET = 0x80;   // NAK ALL
  SYNCDELAY();
  FIFORESET = 0x02;   // Reset EP2
  SYNCDELAY();
  FIFORESET = 0x06;   // Reset EP6
  SYNCDELAY();
  FIFORESET = 0x00;   // Restore
  SYNCDELAY();

#if 1
  OUTPKTEND = 0x82; // 0x80 (SKIP bit) | 0x02 (EP number)
  SYNCDELAY();
  OUTPKTEND = 0x82; 
  SYNCDELAY();
#endif

  // ARM EP2
  EP2BCL = 0x80;
  SYNCDELAY(); 
  EP2BCL = 0x80;

  printf ( "Initialization Done.\n" );

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
  cdc_handle_rx();
  if(d2_led)
  {
    IOA ^= 0x02;
  }
  else
  {
    IOA |= 0x02;
  }
}

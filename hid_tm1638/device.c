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

#include "tm1638.h"
#include "ring_buffer.h"

#define T1_INTERVAL     200

#define SYNCDELAY() SYNCDELAY4

#ifdef DEBUG_FIRMWARE
#include <stdio.h>
#else
#define printf(...)
#endif

////////////////////////////////////////////////////////////////////////////////

/*
 let's implement USB HID here
 USB HID Requirements:
  interface0 - communication class
    EP01    for control
    EP1IN   for report
    EP2     for control (optional)
*/

volatile WORD t1 = 0;
WORD ms_count = 0;
volatile BOOL d2_led = FALSE;
volatile BOOL tick_1ms = FALSE;

static const __code uint8_t ReportDescriptor[] = {
  0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
  0x09, 0x04,                    // USAGE (Joystick)
  0xa1, 0x01,                    // COLLECTION (Application)
        
      // --- INPUT REPORT: Joystick Axes (2 bytes) ---
      0x05, 0x01,                //   USAGE_PAGE (Generic Desktop)
      0x09, 0x30,                //   USAGE (X)
      0x09, 0x31,                //   USAGE (Y)
      0x15, 0x00,                //   LOGICAL_MINIMUM (0)
      0x26, 0xff, 0x00,          //   LOGICAL_MAXIMUM (255)
      0x75, 0x08,                //   REPORT_SIZE (8 bits)
      0x95, 0x02,                //   REPORT_COUNT (2 axes)
      0x81, 0x02,                //   INPUT (Data,Var,Abs)

      // --- INPUT REPORT: TM1638 Buttons (1 byte) ---
      0x05, 0x09,                //   USAGE_PAGE (Button)
      0x19, 0x01,                //   USAGE_MINIMUM (Button 1)
      0x29, 0x08,                //   USAGE_MAXIMUM (Button 8)
      0x15, 0x00,                //   LOGICAL_MINIMUM (0)
      0x25, 0x01,                //   LOGICAL_MAXIMUM (1)
      0x75, 0x01,                //   REPORT_SIZE (1 bit)
      0x95, 0x08,                //   REPORT_COUNT (8 buttons)
      0x81, 0x02,                //   INPUT (Data,Var,Abs)

      // --- OUTPUT REPORT: TM1638 LEDs (1 byte) ---
      0x05, 0x08,                //   USAGE_PAGE (LEDs)
      0x19, 0x01,                //   USAGE_MINIMUM (LED 1)
      0x29, 0x08,                //   USAGE_MAXIMUM (LED 8)
      0x15, 0x00,                //   LOGICAL_MINIMUM (0)
      0x25, 0x01,                //   LOGICAL_MAXIMUM (1)
      0x75, 0x01,                //   REPORT_SIZE (1 bit)
      0x95, 0x08,                //   REPORT_COUNT (8 LEDs)
      0x91, 0x02,                //   OUTPUT (Data,Var,Abs)

      // --- OUTPUT REPORT: TM1638 7-Segments (8 bytes) ---
      0x06, 0x00, 0xff,          //   USAGE_PAGE (Vendor Defined)
      0x09, 0x01,                //   USAGE (Vendor Usage 1)
      0x15, 0x00,                //   LOGICAL_MINIMUM (0)
      0x26, 0xff, 0x00,          //   LOGICAL_MAXIMUM (255)
      0x75, 0x08,                //   REPORT_SIZE (8 bits)
      0x95, 0x08,                //   REPORT_COUNT (8 digits)
      0x91, 0x02,                //   OUTPUT (Data,Var,Abs)

  0xc0                           // END_COLLECTION
};

static const __code uint8_t ReportDescriptorLen  = (uint8_t)sizeof(ReportDescriptor);

void
timer0_isr(void) __interrupt (TF0_ISR)
{
  TH0 = 0xF0;
  TL0 = 0x60;

  ms_count++;
  if(ms_count >= 1000)
  {
    ms_count = 0;
    d2_led = !d2_led;
  }

  t1++;

  if(t1 > T1_INTERVAL)
  {
    t1 = 0;
    IOA ^= 0x01;
  }
  tick_1ms = TRUE;
}

BOOL
handle_get_descriptor(void)
{
#define SC_GET_DESCRIPTOR   0x06
#define GD_REPORT           0x22

  // setup_dat[0] = bmRequestType
  // setup_dat[1] = bRequest (0x06 is GET_DESCRIPTOR)
  // setup_dat[2] = Descriptor Index
  // setup_dat[3] = Descriptor Type (0x22 for HID Report)
  if (SETUPDAT[1] == SC_GET_DESCRIPTOR)
  {
    if (SETUPDAT[3] == GD_REPORT)
    {
#if 0
      // GD_REPORT is 0x22
      // Point the Setup Data Pointer to our descriptor in code memory
      SUDPTRH = MSB(&ReportDescriptor);
      SUDPTRL = LSB(&ReportDescriptor);
#else
      uint16_t requested = SETUPDAT[6] | (SETUPDAT[7] << 8);
      uint16_t sent = 0;
      uint8_t to_send;

      while (sent < requested)
      {
        // Fit current chunk into 64-byte buffer
        to_send = ((requested-sent) > 64) ? 64 : (requested-sent);

        for (uint8_t i = 0; i < to_send; i++)
        {
          EP0BUF[i] = ReportDescriptor[sent + i];
        }

        // Arm and send
        EP0BCH = 0;
        EP0BCL = to_send;

        sent += to_send;

        while (EP0CS & bmEPBUSY);
      }
#endif
      return TRUE; // Handled
    }
  }
  return FALSE; // Let fx2lib handle standard descriptors (Device, Config, etc.)
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
  return TRUE;
}

//********************  INIT ***********************
void
main_init(void)
{
  REVCTL=0;
  SYNCDELAY();
  SETIF48MHZ();

  EP1OUTCFG &= ~bmVALID; 
  SYNCDELAY();
  EP4CFG &= ~bmVALID;
  SYNCDELAY();
  EP6CFG &= ~bmVALID;
  SYNCDELAY();
  EP8CFG &= ~bmVALID;
  SYNCDELAY();

  // 1. Configure EP2-OUT (PC -> Device, for LEDs/Segments)
  // 0xB2 = Valid, Interrupt OUT
  EP2CFG = 0xB2;
  SYNCDELAY();

  // 2. Configure EP1-IN (Device -> PC, for Joystick)
  // 0xB0 = Valid, Interrupt IN
  EP1INCFG = 0xB0;
  SYNCDELAY();

  FIFORESET = 0x80;
  SYNCDELAY();
  FIFORESET = 0x02;
  SYNCDELAY();
  FIFORESET = 0x00;
  SYNCDELAY();

  // Reset EP1IN 
  // Ensuring the busy bit is clear so we can send the first report.
  EP1INCS = 0; 
  SYNCDELAY();

  // Arm EP2OUT
  EP2BCL = 0x80;
  SYNCDELAY();
  EP2BCL = 0x80;
  SYNCDELAY();

  OEA = 0x03;
  TMOD = 0x01; // Timer 0, Mode 1 (16-bit)

  // Set 1ms reload value (0xf060)
  TH0 = 0xf0;
  TL0 = 0x60;
  ENABLE_TIMER0();
  TR0 = 1; // Start Timer 0
           // interrupts are enabled later in main

  tm1638_init();
}

static void
report_button_inputs(void)
{
  if (!(EP1INCS & bmEPBUSY))
  {
    // We only read the buttons when we are actually ready to send them
    EP1INBUF[0] = 128; // X
    EP1INBUF[1] = 128; // Y
    EP1INBUF[2] = tm1638_read_all_buttons();
    EP1INBC     = 3; // Arm the IN transfer
  }
}

__xdata RingBuffer _rb;

static void
process_control_outputs(void)
{
  uint8_t leds;
  uint8_t segs[8];

  // Calculate how many bytes are currently in the ring

  while (rb_available(&_rb) >= 9)
  {
    leds = rb_pop(&_rb);
    segs[0] = rb_pop(&_rb);
    segs[1] = rb_pop(&_rb);
    segs[2] = rb_pop(&_rb);
    segs[3] = rb_pop(&_rb);
    segs[4] = rb_pop(&_rb);
    segs[5] = rb_pop(&_rb);
    segs[6] = rb_pop(&_rb);
    segs[7] = rb_pop(&_rb);

    tm1638_set_leds(leds);
    tm1638_set_segments(segs);
  }
}

static void
check_control_outputs(void)
{
  uint16_t count;

  if(!(EP2468STAT & bmEP2EMPTY))
  {
    count = MAKEWORD(EP2BCH,EP2BCL);

    for(uint16_t i = 0; i < count; i++)
    {
      rb_push(&_rb, EP2FIFOBUF[i]);
    }

    EP2BCL = 0x80;
    SYNCDELAY();
    process_control_outputs();
  }
}

void
main_loop(void)
{
  check_control_outputs();

  if(tick_1ms)
  {
    tick_1ms = FALSE;
    report_button_inputs();
  }

  if(d2_led)
  {
    IOA &= ~0x02;
  }
  else
  {
    IOA |= 0x02;
  }
}

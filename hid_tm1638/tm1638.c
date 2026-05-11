#include <fx2regs.h>
#include <fx2macros.h>

#include "tm1638.h"

#define TM_STB    PA4     // nCS
#define TM_CLK    PA5     // SCLK
#define TM_DIO    PA6     // MOSI/MISO

#define bTM_STB   bmBIT4  // 0x10
#define bTM_CLK   bmBIT5  // 0x20
#define bTM_DIO   bmBIT6  // 0x40

// Simple delay macro for 48MHz timing safety
#define TM_DELAY() __asm nop __endasm;

static inline void
tm1638_write_byte(uint8_t dat)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    TM_CLK = 0;             // Pull clock low

    // Set Data bit (LSB First)
    if (dat & 0x01)
    {
      TM_DIO = 1;
    }
    else
    {
      TM_DIO = 0;
    }

    TM_DELAY();             // Give data time to stabilize
    TM_CLK = 1;             // Pull clock high (TM1638 captures here)
    TM_DELAY();

    dat >>= 1;              // Shift to next bit
  }
}

static inline uint8_t
tm1638_read_byte(void)
{
  uint8_t i;
  uint8_t dat = 0;

  // Switch DIO to Input
  OEA &= ~bTM_DIO; 

  // Note: The TM1638 needs a small gap here if this is 
  // called immediately after a write command.
  TM_DELAY();

  for (i = 0; i < 8; i++)
  {
    dat >>= 1;              // Shift to make room for LSB-first data

    TM_CLK = 0;             // Pull clock low
    TM_DELAY();             // Wait for TM1638 to put data on DIO

    if (TM_DIO) {           
      dat |= 0x80;        // Set the bit
    }

    TM_CLK = 1;             // Pull clock high
    TM_DELAY();
  }

  // Switch DIO back to Output so we don't leave the bus floating
  OEA |= bTM_DIO; 

  return dat;
}

static void
tm1638_send_command(uint8_t cmd)
{
  TM_STB = 0;
  tm1638_write_byte(cmd);
  TM_STB = 1;
}

uint8_t
tm1638_read_all_buttons(void)
{
  uint8_t keys = 0;
  TM_STB = 0;
  tm1638_write_byte(0x42); // Command: Read Keys
                           //
  for (uint8_t i = 0; i < 4; i++)
  {
    uint8_t received = tm1638_read_byte();
    // TM1638 K1-K4 are in bits 0 of each byte, K5-K8 are in bits 4
    if (received & 0x01) keys |= (1 << i);       
    if (received & 0x10) keys |= (1 << (i + 4)); 
  }
  TM_STB = 1;
  return keys;
}

void
tm1638_set_leds(uint8_t led_mask)
{
  uint8_t i;

  // Set to Fixed Address Mode (Command 0x44)
  tm1638_send_command(0x44);

  for (i = 0; i < 8; i++)
  {
    TM_STB = 0;
    // Address for LED (i) is 0xC1 + (i * 2)
    tm1638_write_byte(0xC1 + (i * 2));

    // Write 1 to turn on, 0 to turn off
    if (led_mask & (1 << i)) {
      tm1638_write_byte(0x01);
    } else {
      tm1638_write_byte(0x00);
    }
    TM_STB = 1;
  }
}

/**
 * tm1638_set_segments
 * @segments: Pointer to an 8-byte array. 
 * Each byte defines segments A-G and the decimal point (DP).
 * 
 * Segment mapping in one byte (standard):
 * Bit:  7  6  5  4  3  2  1  0
 * Part: DP G  F  E  D  C  B  A
 */
void
tm1638_set_segments(const uint8_t *segments)
{
  uint8_t i;

  // Set to Fixed Address Mode
  tm1638_send_command(0x44);

  for (i = 0; i < 8; i++)
  {
    TM_STB = 0;
    // 7-segment addresses are the even ones: C0, C2, C4...
    tm1638_write_byte(0xC0 + (i * 2));

    // Write the segment pattern
    tm1638_write_byte(segments[i]);
    TM_STB = 1;
  }
}

void
tm1638_init(void)
{
  uint8_t i;

  // 1. Hardware Pin Config
  OEA |= (bTM_STB | bTM_CLK | bTM_DIO); // All outputs initially
  TM_STB = 1;
  TM_CLK = 1;
  TM_DIO = 0;

  // 2. Set Data Mode: Auto-increment address (0x40)
  TM_STB = 0;
  tm1638_write_byte(0x40);
  TM_STB = 1;

  // 3. Clear All 16 Registers (0xC0 to 0xCF)
  TM_STB = 0;
  tm1638_write_byte(0xC0); // Starting address
  for (i = 0; i < 16; i++) {
    tm1638_write_byte(0x00);
  }
  TM_STB = 1;

  // 4. Activate Display and set Brightness (0x88 = ON, min; 0x8F = ON, max)
  TM_STB = 0;
  tm1638_write_byte(0x8F);
  TM_STB = 1;
}

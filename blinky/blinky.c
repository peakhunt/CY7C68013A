#include <fx2regs.h>
#include "delay.h"

#define TIMER1_INTERVAL     1163
#define TIMER2_INTERVAL     11317

void
main(void)
{
  // 1. Set PA0 and PA1 as outputs
  OEA = 0x03;

  // 2. State trackers for independent blinking
  unsigned int timer1 = 0;
  unsigned int timer2 = 0;

  while(1)
  {
    // Precise base delay of 1ms
    delay(1); 

    // LED 1 (PA0) - Toggles every 200ms
    timer1++;
    if (timer1 >= TIMER1_INTERVAL)
    {
      IOA ^= 0x01;
      timer1 = 0;
    }

    // LED 2 (PA1) - Toggles every 750ms
    timer2++;
    if (timer2 >= TIMER2_INTERVAL)
    {
      IOA ^= 0x02;
      timer2 = 0;
    }
  }
}

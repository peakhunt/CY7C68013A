#pragma once

#include <stdint.h>

extern void tm1638_init(void);
extern void tm1638_set_segments(const uint8_t *segments);
extern void tm1638_set_leds(uint8_t led_mask);
extern uint8_t tm1638_read_all_buttons(void);

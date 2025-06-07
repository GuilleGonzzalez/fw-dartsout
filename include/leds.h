#ifndef __LEDS_H
#define __LEDS_H


void leds_init();
void leds_set_pixel_color(int i, uint32_t color);
uint32_t leds_color(int r, int g, int b, int w);
void leds_show();
void leds_set_color(uint32_t color);
void leds_color_wipe(uint32_t color, int wait);
void leds_rainbow(uint32_t firstPixelHue);
void leds_white_over_rainbow(int white_speed, int white_len);
void leds_pulse_white(uint8_t wait);
void leds_rainbow_fade_to_white(int wait, int rainbow_loops, int white_loops);

#endif // __LEDS_H

/**
 * C Library for SSD1306 OLED Display
 * Author: Pavel Koltyshev
 * (c) 2025
*/

#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"
#include "ssd1306_def.h"

ssd1306_config_t ssd1306_get_default_config();
ssd1306_t ssd1306_create(i2c_inst_t* i2c_inst, uint8_t i2c_address);
bool ssd1306_init(ssd1306_t* ssd1306, const ssd1306_config_t* config);
bool ssd1306_set_contrast(ssd1306_t* ssd1306, uint8_t contrast);
bool ssd1306_set_inverse(ssd1306_t* ssd1306, bool value);
bool ssd1306_display_on(ssd1306_t* ssd1306);
bool ssd1306_display_off(ssd1306_t* ssd1306);
bool ssd1306_clear_display(ssd1306_t* ssd1306);
void ssd1306_set_font(ssd1306_t* ssd1306, const font_t* font);
bool ssd1306_print(ssd1306_t* ssd1306, const char* text, uint8_t start_x, uint8_t start_y);
bool ssd1306_draw_bitmap(ssd1306_t* ssd1306, const bitmap_t* bitmap, uint8_t start_x, uint8_t start_y);
bool ssd1306_show(ssd1306_t* ssd1306);


#endif // SSD1306_H

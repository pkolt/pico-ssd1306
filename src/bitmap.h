/**
 * C Library for SSD1306 OLED Display
 * Author: Pavel Koltyshev
 * (c) 2025
*/

#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>

typedef struct {
    uint8_t width;
    uint8_t height;
    const uint8_t* data;
} bitmap_t;

#endif // BITMAP_H

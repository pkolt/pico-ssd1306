/**
 * C Library for SSD1306 OLED Display
 * Author: Pavel Koltyshev
 * (c) 2025
*/

#ifndef FONT_H
#define FONT_H

#include <stdint.h>

typedef struct {
    uint16_t start;
    uint16_t end;
    uint16_t symbols_count;
    const uint8_t* symbols;
    const uint32_t* offsets;
    const uint8_t* widths; // NULL for monospaced fonts
} font_subset_t;

typedef struct {
    uint8_t width;
    uint8_t height;
    uint8_t letter_spacing;
    uint8_t word_spacing;
    uint16_t subsets_count;
    const font_subset_t* subsets;
} font_t;

#endif // FONT_H

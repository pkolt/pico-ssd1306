/**
 * C Library for SSD1306 OLED Display
 * Author: Pavel Koltyshev
 * (c) 2025
*/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ssd1306_def.h"

static bool i2c_check(int result) {
    return result >= 0;
}

static bool ssd1306_send_command(ssd1306_t* ssd1306, uint8_t command) {
    return i2c_check(i2c_write_blocking(ssd1306->i2c_inst, ssd1306->i2c_address, (uint8_t[]){SSD1306_SEND_COMMAND, command}, 2, false));
}

static bool ssd1306_send_command_value(ssd1306_t* ssd1306, uint8_t command, uint8_t value) {
    return i2c_check(i2c_write_blocking(ssd1306->i2c_inst, ssd1306->i2c_address, (uint8_t[]){SSD1306_SEND_COMMAND, command, value}, 3, false));
}

static bool ssd1306_send_command_2_values(ssd1306_t* ssd1306, uint8_t command, uint8_t value1, uint8_t value2) {
    return i2c_check(i2c_write_blocking(ssd1306->i2c_inst, ssd1306->i2c_address, (uint8_t[]){SSD1306_SEND_COMMAND, command, value1, value2}, 4, false));
}

/**
 * Set Contrast Control
 * @param contrast (1-255) 0x7F = 127 (RESET)
*/
bool ssd1306_set_contrast(ssd1306_t* ssd1306, uint8_t contrast) {
    return ssd1306_send_command_value(ssd1306, SSD1306_CONTRAST_COMMAND, contrast);
}

/**
 * Inverse Display
 * @param ssd1306
 * @param enabled (RESET = false)
*/
bool ssd1306_set_inverse(ssd1306_t* ssd1306, bool enabled) {
    return ssd1306_send_command(ssd1306, enabled ? SSD1306_DISPLAY_INVERSE_COMMAND : SSD1306_DISPLAY_NORMAL_COMMAND);
}

/**
 * Display ON
*/
bool ssd1306_display_on(ssd1306_t* ssd1306) {
    return ssd1306_send_command(ssd1306, SSD1306_DISPLAY_ON_COMMAND);
}

/**
 * Display OFF (sleep mode) (RESET)
*/
bool ssd1306_display_off(ssd1306_t* ssd1306) {
    return ssd1306_send_command(ssd1306, SSD1306_DISPLAY_OFF_COMMAND);
}

/**
 * Validate Page
 * @param page (0-7)
*/
static bool is_valid_page(uint8_t page) {
    return page >= SSD1306_PAGE_START_ADDRESS && page <= SSD1306_PAGE_END_ADDRESS;
}

/**
 * Validate Column
 * @param column (0-127)
*/
static bool is_valid_column(uint8_t column) {
    return column >= SSD1306_COLUMN_START_ADDRESS && column <= SSD1306_COLUMN_END_ADDRESS;
}

/**
 * Set area for draw
 * @param ssd1306
 * @param start_page (0-7)
 * @param end_page (0-7)
 * @param start_column (0-127)
 * @param end_column (0-127)
*/
static bool ssd1306_set_area(ssd1306_t* ssd1306, uint8_t start_page, uint8_t end_page, uint8_t start_column, uint8_t end_column) {
    if (!is_valid_page(start_page) || 
        !is_valid_page(end_page) || 
        !is_valid_column(start_column) || 
        !is_valid_column(end_column)) {
        return false;
    }

    bool is_ok = ssd1306_send_command_2_values(ssd1306, SSD1306_PAGE_START_END_ADDRESS_COMMAND, start_page, end_page);
    is_ok = is_ok && ssd1306_send_command_2_values(ssd1306, SSD1306_COLUMN_START_END_ADDRESS_COMMAND, start_column, end_column);

    return is_ok;
}

bool ssd1306_clear_display(ssd1306_t* ssd1306) {
    memset(ssd1306->buffer + 1, 0, SSD1306_DISPLAY_BYTES);
    return true;
}

ssd1306_config_t ssd1306_get_default_config() {
    ssd1306_config_t settings = {};

    // 1. Fundamental Command
    settings.contrast = SSD1306_CONTRAST_DEFAULT;
    settings.inverse = false;

    // 2. Scrolling Command
    // ...

    // 3. Addressing Setting Command
    settings.memory_addressing_mode = SSD1306_MEMORY_ADDRESSING_MODE_HORIZONTAL;

    // 4. Hardware Configuration Command
    settings.segment_re_map_inverse = true;
    settings.mux_ratio = SSD1306_MUX_RATIO_MAX;
    settings.com_output_scan_direction_remapped = true;
    settings.com_alt_pin_config = true;
    settings.com_disable_left_right_remap = true;

    // 5. Timing & Driving Scheme Setting Command
    settings.divide_ratio = SSD1306_DISPLAY_CLOCK_DIVIDE_RATIO_MIN;
    settings.oscillator_frequency = SSD1306_DISPLAY_CLOCK_OSCILLATOR_FREQUENCY_MAX;
    settings.pre_charge_period_phase_1 = SSD1306_PRE_CHARGE_PERIOD_PHASE1_DEFAULT;
    settings.pre_charge_period_phase_2 = SSD1306_PRE_CHARGE_PERIOD_PHASE2_DEFAULT;
    settings.vcomh_deselect_level = SSD1306_VCOMH_DESELECT_LEVEL1;

    // 6. Advance Graphic Command
    settings.fade_out_blinking_mode = SSD1306_FADE_OUT_BLINKING_DISABLE;
    settings.fade_out_time_interval = SSD1306_FADE_OUT_BLINKING_TIME_INTERVAL_MIN;
    settings.zoom = false;

    // 7. Charge Pump Command
    settings.charge_pump = true;

    return settings;
}

ssd1306_t ssd1306_create(i2c_inst_t* i2c_inst, uint8_t i2c_address) {
    ssd1306_t ssd1306 = { .i2c_inst = i2c_inst, .i2c_address = i2c_address, .font = NULL };
    return ssd1306;
};

bool ssd1306_init(ssd1306_t* ssd1306, const ssd1306_config_t* config) {
    ssd1306->buffer[0] = SSD1306_SEND_DATA;

    memset(ssd1306->buffer + 1, 0, SSD1306_DISPLAY_BYTES);

    uint8_t commands[31];
    uint8_t i = 0;
    commands[i++] = SSD1306_SEND_COMMAND;

    commands[i++] = config->com_output_scan_direction_remapped ? SSD1306_COM_OUTPUT_SCAN_DIRECTION_REMAPPED_COMMAND : SSD1306_COM_OUTPUT_SCAN_DIRECTION_NORMAL_COMMAND;

    if (config->mux_ratio < SSD1306_MUX_RATIO_MIN || config->mux_ratio > SSD1306_MUX_RATIO_MAX) return false;
    commands[i++] = SSD1306_MUX_RATIO_COMMAND;
    commands[i++] = config->mux_ratio;

    if (config->divide_ratio < SSD1306_DISPLAY_CLOCK_DIVIDE_RATIO_MIN ||
        config->divide_ratio > SSD1306_DISPLAY_CLOCK_DIVIDE_RATIO_MAX ||
        config->oscillator_frequency < SSD1306_DISPLAY_CLOCK_OSCILLATOR_FREQUENCY_MIN ||
        config->oscillator_frequency > SSD1306_DISPLAY_CLOCK_OSCILLATOR_FREQUENCY_MAX) {
        return false;
    }
    commands[i++] = SSD1306_DISPLAY_CLOCK_DIVIDE_COMMAND;
    commands[i++] = (config->divide_ratio) | (config->oscillator_frequency << 4);
    
    commands[i++] = config->inverse ? SSD1306_DISPLAY_INVERSE_COMMAND : SSD1306_DISPLAY_NORMAL_COMMAND;
    
    commands[i++] = SSD1306_CONTRAST_COMMAND;
    commands[i++] = config->contrast;

    if ((config->fade_out_blinking_mode != SSD1306_FADE_OUT_BLINKING_DISABLE && config->fade_out_blinking_mode != SSD1306_FADE_OUT_MODE && config->fade_out_blinking_mode != SSD1306_BLINKING_MODE) ||
        (config->fade_out_time_interval < SSD1306_FADE_OUT_BLINKING_TIME_INTERVAL_MIN || config->fade_out_time_interval > SSD1306_FADE_OUT_BLINKING_TIME_INTERVAL_MAX)) {
        return false;
    }
    commands[i++] = SSD1306_FADE_OUT_BLINKING_COMMAND;
    commands[i++] = config->fade_out_blinking_mode | config->fade_out_time_interval;
    
    commands[i++] = SSD1306_ZOOM_IN_COMMAND;
    commands[i++] = config->zoom ? SSD1306_ZOOM_IN_ENABLE : SSD1306_ZOOM_IN_DISABLE;
    
    commands[i++] = SSD1306_DISPLAY_OFFSET_COMMAND;
    commands[i++] = SSD1306_DISPLAY_OFFSET_MIN;
    
    commands[i++] = SSD1306_MEMORY_ADDRESSING_MODE_COMMAND;
    commands[i++] = config->memory_addressing_mode;
    
    if (config->pre_charge_period_phase_1 < SSD1306_PRE_CHARGE_PERIOD_PHASE_MIN ||
        config->pre_charge_period_phase_1 > SSD1306_PRE_CHARGE_PERIOD_PHASE_MAX ||
        config->pre_charge_period_phase_2 < SSD1306_PRE_CHARGE_PERIOD_PHASE_MIN ||
        config->pre_charge_period_phase_2 > SSD1306_PRE_CHARGE_PERIOD_PHASE_MAX) {
        return false;
    }
    commands[i++] = SSD1306_PRE_CHARGE_PERIOD_COMMAND;
    commands[i++] = (config->pre_charge_period_phase_1 << 4) | config->pre_charge_period_phase_2;
    
    if (config->vcomh_deselect_level != SSD1306_VCOMH_DESELECT_LEVEL0 && config->vcomh_deselect_level != SSD1306_VCOMH_DESELECT_LEVEL1 && config->vcomh_deselect_level != SSD1306_VCOMH_DESELECT_LEVEL2) {
        return false;
    }
    commands[i++] = SSD1306_VCOMH_DESELECT_LEVEL_COMMAND;
    commands[i++] = config->vcomh_deselect_level;
    
    const uint8_t com_pins_val1 = config->com_alt_pin_config ? SSD1306_COM_PINS_HARDWARE_CONFIG_ALTERNATIVE_COM_PIN : SSD1306_COM_PINS_HARDWARE_CONFIG_SEQUENTIAL_COM_PIN;
    const uint8_t com_pins_val2 = config->com_disable_left_right_remap ? SSD1306_COM_PINS_HARDWARE_CONFIG_DISABLE_REMAP : SSD1306_COM_PINS_HARDWARE_CONFIG_ENABLE_REMAP;
    commands[i++] = SSD1306_COM_PINS_HARDWARE_CONFIG_COMMAND;
    commands[i++] = com_pins_val1 | com_pins_val2;
    
    commands[i++] = config->segment_re_map_inverse ? SSD1306_SEGMENT_RE_MAP_INVERSE_COMMAND : SSD1306_SEGMENT_RE_MAP_NORMAL_COMMAND;
    
    commands[i++] = SSD1306_CHARGE_PUMP_COMMAND;
    commands[i++] = config->charge_pump ? SSD1306_CHARGE_PUMP_ENABLE : SSD1306_CHARGE_PUMP_DISABLE;
    
    commands[i++] = SSD1306_DISPLAY_ON_COMMAND;

    return i2c_check(i2c_write_blocking(ssd1306->i2c_inst, ssd1306->i2c_address, commands, i, false));
}

static bool _ssd1306_draw_bitmap_internal(ssd1306_t* ssd1306, const uint8_t* bitmap, uint32_t offset, uint8_t width, uint8_t height, uint8_t start_x, uint8_t start_y) {
    if (width == 0 || height == 0) {
        return false;
    }

    const uint8_t *bitmap_data = &bitmap[offset];
    const uint16_t bitmap_bytes_per_row = (width + 7) / 8;

    for (uint16_t y_in_bitmap = 0; y_in_bitmap < height; y_in_bitmap++) {
        for (uint16_t x_in_bitmap = 0; x_in_bitmap < width; x_in_bitmap++) {
            
            uint16_t visual_x = start_x + x_in_bitmap;
            uint16_t visual_y = start_y + y_in_bitmap;

            if (visual_x >= SSD1306_WIDTH || visual_y >= SSD1306_HEIGHT) {
                continue;
            }

            uint16_t byte_index = y_in_bitmap * bitmap_bytes_per_row + (x_in_bitmap / 8);

            uint8_t src_byte = bitmap_data[byte_index];
            bool pixel_is_on = (src_byte >> (x_in_bitmap % 8)) & 1;

            uint16_t buffer_index = (visual_y / 8) * SSD1306_WIDTH + visual_x;
            uint8_t buffer_bit = visual_y % 8;

            if (pixel_is_on) {
                ssd1306->buffer[buffer_index + 1] |= (1 << buffer_bit);
            } else {
                ssd1306->buffer[buffer_index + 1] &= ~(1 << buffer_bit);
            }
        }
    }
    return true;
}

bool ssd1306_draw_bitmap(ssd1306_t* ssd1306, const bitmap_t* bitmap, uint8_t start_x, uint8_t start_y) {
    if (bitmap == NULL || bitmap->data == NULL) {
        return false;
    }
    return _ssd1306_draw_bitmap_internal(ssd1306, bitmap->data, 0, bitmap->width, bitmap->height, start_x, start_y);
}

void ssd1306_set_font(ssd1306_t* ssd1306, const font_t* font) {
    ssd1306->font = font;
}

bool ssd1306_print(ssd1306_t* ssd1306, const char* text, uint8_t start_x, uint8_t start_y) {
    const font_t* font = ssd1306->font;
    if (font == NULL || text == NULL) {
        return false;
    }

    uint8_t current_x = start_x;
    uint16_t codepoint;

    while (*text != '\0') {
        if (current_x >= SSD1306_WIDTH) {
            break;
        }

        uint8_t first_byte = (uint8_t)*text;
        int bytes_to_advance = 1; // Default to 1 byte for ASCII or error

        if (first_byte < 0x80) { // 1-byte ASCII character
            codepoint = first_byte;
        } else if ((first_byte & 0xE0) == 0xC0) { // 2-byte UTF-8 sequence (U+0080 to U+07FF)
            if (*(text + 1) == '\0') { // Incomplete sequence
                break;
            }
            uint8_t second_byte = (uint8_t)*(text + 1);
            if ((second_byte & 0xC0) != 0x80) { // Invalid second byte
                codepoint = '?'; // Placeholder
            } else {
                codepoint = ((first_byte & 0x1F) << 6) | (second_byte & 0x3F);
                bytes_to_advance = 2;
            }
        } else if ((first_byte & 0xF0) == 0xE0) { // 3-byte UTF-8 sequence (U+0800 to U+FFFF)
            if (*(text + 1) == '\0' || *(text + 2) == '\0') { // Incomplete sequence
                break;
            }
            uint8_t second_byte = (uint8_t)*(text + 1);
            uint8_t third_byte = (uint8_t)*(text + 2);
            if (((second_byte & 0xC0) != 0x80) || ((third_byte & 0xC0) != 0x80)) { // Invalid bytes
                codepoint = '?'; // Placeholder
            } else {
                codepoint = ((first_byte & 0x0F) << 12) | ((second_byte & 0x3F) << 6) | (third_byte & 0x3F);
                bytes_to_advance = 3;
            }
        } else if ((first_byte & 0xF8) == 0xF0) { // 4-byte UTF-8 sequence (U+10000 to U+10FFFF)
            // Codepoints above U+FFFF cannot be represented by uint16_t, so we'll treat them as unsupported
            if (*(text + 1) == '\0' || *(text + 2) == '\0' || *(text + 3) == '\0') { // Incomplete sequence
                break;
            }
            // For 4-byte UTF-8, we advance the pointer but use a placeholder as uint16_t cannot hold it.
            codepoint = '?'; 
            bytes_to_advance = 4;
        } else { // Invalid UTF-8 start byte or other unsupported sequences
            codepoint = '?'; // Placeholder for invalid character
        }

        // Advance text pointer
        text += bytes_to_advance;

        if (codepoint == ' ') {
            current_x += font->word_spacing;
            continue;
        }

        uint8_t width = font->width; // Default width
        bool symbol_found = false;

        for (size_t i = 0; i < font->subsets_count; i++) {
            const font_subset_t* subset = &font->subsets[i];
            if (codepoint >= subset->start && codepoint <= subset->end) {
                size_t char_index = codepoint - subset->start;
                if (char_index < subset->symbols_count) {
                    uint32_t offset = subset->offsets[char_index];
                    if (subset->widths) { // Variable width font
                        width = subset->widths[char_index];
                    }

                    if (!_ssd1306_draw_bitmap_internal(ssd1306, subset->symbols, offset, width, font->height, current_x, start_y)) {
                        return false; // Stop if drawing fails
                    }
                    symbol_found = true;
                }
                break;
            }
        }
        
        // If symbol not found, and it's not a placeholder already,
        // you might want to print a default '?' or skip advancement.
        // For now, we always advance current_x.
        current_x += width + font->letter_spacing;
    }

    return true;
}

bool ssd1306_show(ssd1306_t* ssd1306) {
    bool is_ok = ssd1306_set_area(ssd1306, SSD1306_PAGE_START_ADDRESS, SSD1306_PAGE_END_ADDRESS, SSD1306_COLUMN_START_ADDRESS, SSD1306_COLUMN_END_ADDRESS);
    return is_ok && i2c_check(i2c_write_blocking(ssd1306->i2c_inst, ssd1306->i2c_address, ssd1306->buffer, SSD1306_DISPLAY_BYTES + 1, false));
}

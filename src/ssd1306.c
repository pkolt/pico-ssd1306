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
#include "ssd1306.h"

// Maximum bytes in the SSD1306 init command sequence, including leading control byte.
#define SSD1306_INIT_COMMANDS_CAPACITY 30

static bool i2c_write_exact(int result, size_t expected_len) {
    return result == (int)expected_len;
}

static uint16_t ssd1306_get_display_bytes(const ssd1306_t* ssd1306) {
    return (uint16_t)(((uint16_t)ssd1306->width * (uint16_t)ssd1306->height) / SSD1306_BITS_IN_BYTE);
}

static bool ssd1306_has_valid_geometry(const ssd1306_t* ssd1306) {
    return ssd1306 != NULL && ssd1306->width > 0 && ssd1306->height > 0;
}

static bool ssd1306_is_ready(const ssd1306_t* ssd1306) {
    return ssd1306_has_valid_geometry(ssd1306) && ssd1306->buffer != NULL && ssd1306->buffer_size > 1;
}

static bool ssd1306_send_command(ssd1306_t* ssd1306, uint8_t command) {
    return i2c_write_exact(i2c_write_timeout_us(ssd1306->i2c_inst, ssd1306->i2c_address, (uint8_t[]){SSD1306_SEND_COMMAND, command}, 2, false, SSD1306_I2C_TIMEOUT_US), 2);
}

static bool ssd1306_send_command_value(ssd1306_t* ssd1306, uint8_t command, uint8_t value) {
    return i2c_write_exact(i2c_write_timeout_us(ssd1306->i2c_inst, ssd1306->i2c_address, (uint8_t[]){SSD1306_SEND_COMMAND, command, value}, 3, false, SSD1306_I2C_TIMEOUT_US), 3);
}

static bool ssd1306_send_command_2_values(ssd1306_t* ssd1306, uint8_t command, uint8_t value1, uint8_t value2) {
    return i2c_write_exact(i2c_write_timeout_us(ssd1306->i2c_inst, ssd1306->i2c_address, (uint8_t[]){SSD1306_SEND_COMMAND, command, value1, value2}, 4, false, SSD1306_I2C_TIMEOUT_US), 4);
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
static bool is_valid_page(uint8_t page, uint8_t max_page) {
    return page >= SSD1306_PAGE_START_ADDRESS && page <= max_page;
}

/**
 * Validate Column
 * @param column (0-127)
*/
static bool is_valid_column(uint8_t column, uint8_t max_column) {
    return column >= SSD1306_COLUMN_START_ADDRESS && column <= max_column;
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
    if (!ssd1306_has_valid_geometry(ssd1306)) {
        return false;
    }

    const uint8_t max_page = (ssd1306->height / SSD1306_BITS_PER_COLUMN) - 1;
    const uint8_t max_column = ssd1306->width - 1;

    if (max_page > SSD1306_PAGE_END_ADDRESS || max_column > SSD1306_COLUMN_END_ADDRESS) {
        return false;
    }

    if (start_page > end_page || start_column > end_column) {
        return false;
    }

    if (!is_valid_page(start_page, max_page) || 
        !is_valid_page(end_page, max_page) || 
        !is_valid_column(start_column, max_column) || 
        !is_valid_column(end_column, max_column)) {
        return false;
    }

    bool is_ok = ssd1306_send_command_2_values(ssd1306, SSD1306_PAGE_START_END_ADDRESS_COMMAND, start_page, end_page);
    is_ok = is_ok && ssd1306_send_command_2_values(ssd1306, SSD1306_COLUMN_START_END_ADDRESS_COMMAND, start_column, end_column);

    return is_ok;
}

bool ssd1306_clear_display(ssd1306_t* ssd1306) {
    if (!ssd1306_is_ready(ssd1306)) {
        return false;
    }
    memset(ssd1306->buffer + 1, 0, ssd1306->buffer_size - 1);
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

ssd1306_t ssd1306_create(i2c_inst_t* i2c_inst, uint8_t i2c_address, ssd1306_display_size_t display_size) {
    ssd1306_t ssd1306 = {
        .i2c_inst = i2c_inst,
        .i2c_address = i2c_address,
        .width = 0,
        .height = 0,
        .font = NULL,
        .buffer_size = 0,
        .buffer = NULL
    };

    switch (display_size) {
        case SSD1306_DISPLAY_SIZE_128x64:
            ssd1306.width = 128;
            ssd1306.height = 64;
            break;
        case SSD1306_DISPLAY_SIZE_128x32:
            ssd1306.width = 128;
            ssd1306.height = 32;
            break;
        default:
            return ssd1306;
    }

    const uint16_t display_bytes = ssd1306_get_display_bytes(&ssd1306);
    // +1 for the leading I2C control byte (SSD1306_SEND_DATA) before framebuffer bytes.
    ssd1306.buffer_size = display_bytes + 1;
    ssd1306.buffer = (uint8_t*)malloc(ssd1306.buffer_size);
    if (ssd1306.buffer == NULL) {
        ssd1306.buffer_size = 0;
        return ssd1306;
    }

    ssd1306.buffer[0] = SSD1306_SEND_DATA;
    memset(ssd1306.buffer + 1, 0, display_bytes);

    return ssd1306;
};

bool ssd1306_init(ssd1306_t* ssd1306, const ssd1306_config_t* config) {
    if (config == NULL || !ssd1306_is_ready(ssd1306)) {
        return false;
    }

    ssd1306->buffer[0] = SSD1306_SEND_DATA;
    memset(ssd1306->buffer + 1, 0, ssd1306->buffer_size - 1);

    uint8_t commands[SSD1306_INIT_COMMANDS_CAPACITY];
    uint8_t i = 0;
    commands[i++] = SSD1306_SEND_COMMAND;

    // Ensure deterministic init even without a dedicated RESET pin.
    commands[i++] = SSD1306_DISPLAY_OFF_COMMAND;

    // Match controller scan geometry to the selected panel size.
    const uint8_t geometry_mux_ratio = ssd1306->height - 1;
    const bool geometry_com_alt_pin_config = (ssd1306->height > 32);

    commands[i++] = config->com_output_scan_direction_remapped ? SSD1306_COM_OUTPUT_SCAN_DIRECTION_REMAPPED_COMMAND : SSD1306_COM_OUTPUT_SCAN_DIRECTION_NORMAL_COMMAND;

    if (geometry_mux_ratio < SSD1306_MUX_RATIO_MIN || geometry_mux_ratio > SSD1306_MUX_RATIO_MAX) return false;
    commands[i++] = SSD1306_MUX_RATIO_COMMAND;
    commands[i++] = geometry_mux_ratio;

    if (config->divide_ratio < SSD1306_DISPLAY_CLOCK_DIVIDE_RATIO_MIN ||
        config->divide_ratio > SSD1306_DISPLAY_CLOCK_DIVIDE_RATIO_MAX ||
        config->oscillator_frequency < SSD1306_DISPLAY_CLOCK_OSCILLATOR_FREQUENCY_MIN ||
        config->oscillator_frequency > SSD1306_DISPLAY_CLOCK_OSCILLATOR_FREQUENCY_MAX) {
        return false;
    }
    commands[i++] = SSD1306_DISPLAY_CLOCK_DIVIDE_COMMAND;
    commands[i++] = (config->divide_ratio) | (config->oscillator_frequency << 4);
    
    commands[i++] = config->inverse ? SSD1306_DISPLAY_INVERSE_COMMAND : SSD1306_DISPLAY_NORMAL_COMMAND;
    commands[i++] = SSD1306_ENTIRE_DISPLAY_ON_COMMAND; // A4: disable "entire display ON" override and render RAM again (opposite of A5)
    commands[i++] = SSD1306_DISPLAY_START_LINE_COMMAND; // Start line = 0
    
    commands[i++] = SSD1306_CONTRAST_COMMAND;
    commands[i++] = config->contrast;

    if (config->fade_out_time_interval < SSD1306_FADE_OUT_BLINKING_TIME_INTERVAL_MIN ||
        config->fade_out_time_interval > SSD1306_FADE_OUT_BLINKING_TIME_INTERVAL_MAX) {
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
    
    commands[i++] = SSD1306_VCOMH_DESELECT_LEVEL_COMMAND;
    commands[i++] = config->vcomh_deselect_level;
    
    const uint8_t com_pins_val1 = geometry_com_alt_pin_config ? SSD1306_COM_PINS_HARDWARE_CONFIG_ALTERNATIVE_COM_PIN : SSD1306_COM_PINS_HARDWARE_CONFIG_SEQUENTIAL_COM_PIN;
    const uint8_t com_pins_val2 = config->com_disable_left_right_remap ? SSD1306_COM_PINS_HARDWARE_CONFIG_DISABLE_REMAP : SSD1306_COM_PINS_HARDWARE_CONFIG_ENABLE_REMAP;
    commands[i++] = SSD1306_COM_PINS_HARDWARE_CONFIG_COMMAND;
    commands[i++] = com_pins_val1 | com_pins_val2;
    
    commands[i++] = config->segment_re_map_inverse ? SSD1306_SEGMENT_RE_MAP_INVERSE_COMMAND : SSD1306_SEGMENT_RE_MAP_NORMAL_COMMAND;
    
    commands[i++] = SSD1306_CHARGE_PUMP_COMMAND;
    commands[i++] = config->charge_pump ? SSD1306_CHARGE_PUMP_ENABLE : SSD1306_CHARGE_PUMP_DISABLE;
    
    commands[i++] = SSD1306_DISPLAY_ON_COMMAND;

    return i2c_write_exact(i2c_write_timeout_us(ssd1306->i2c_inst, ssd1306->i2c_address, commands, i, false, SSD1306_I2C_TIMEOUT_US), i);
}

static bool _ssd1306_draw_bitmap_internal(ssd1306_t* ssd1306, const uint8_t* bitmap, uint32_t offset, uint8_t width, uint8_t height, uint8_t start_x, uint8_t start_y) {
    if (!ssd1306_is_ready(ssd1306) || width == 0 || height == 0) {
        return false;
    }

    const uint8_t *bitmap_data = &bitmap[offset];
    const uint16_t bitmap_bytes_per_row = (width + 7) / 8;
    const uint16_t display_width = ssd1306->width;
    const uint16_t display_height = ssd1306->height;

    // Fast reject when bitmap starts fully outside visible area.
    if (start_x >= display_width || start_y >= display_height) {
        return true;
    }

    // Clip the source rectangle once, so the hot loops below stay branch-light.
    const bool clipped_x = ((uint16_t)start_x + width > display_width);
    const bool clipped_y = ((uint16_t)start_y + height > display_height);
    const uint16_t draw_width = clipped_x ? (display_width - start_x) : width;
    const uint16_t draw_height = clipped_y ? (display_height - start_y) : height;

    // Split each source row into full 8-pixel chunks and optional tail bits.
    const uint16_t full_bytes = draw_width >> 3;
    const uint16_t tail_bits = draw_width & 0x07;

    for (uint16_t y_in_bitmap = 0; y_in_bitmap < draw_height; y_in_bitmap++) {
        const uint16_t visual_y = start_y + y_in_bitmap;
        // SSD1306 framebuffer is page-based: one byte stores 8 vertical pixels in a column.
        const uint16_t page = visual_y >> 3;
        const uint8_t buffer_bit = (uint8_t)(visual_y & 0x07);
        const uint8_t bitmask = (uint8_t)(1u << buffer_bit);
        const uint8_t inv_bitmask = (uint8_t)~bitmask;
        const uint16_t row_base = y_in_bitmap * bitmap_bytes_per_row;
        const uint16_t buffer_row_base = (page * display_width) + start_x + 1; // +1 skips SSD1306_SEND_DATA control byte

        // Fast path for complete bytes from source row.
        for (uint16_t src_byte_idx = 0; src_byte_idx < full_bytes; src_byte_idx++) {
            const uint8_t src_byte = bitmap_data[row_base + src_byte_idx];
            const uint16_t buffer_index = buffer_row_base + (src_byte_idx << 3);
            ssd1306->buffer[buffer_index + 0] = (ssd1306->buffer[buffer_index + 0] & inv_bitmask) | ((src_byte & (1u << 0)) ? bitmask : 0u);
            ssd1306->buffer[buffer_index + 1] = (ssd1306->buffer[buffer_index + 1] & inv_bitmask) | ((src_byte & (1u << 1)) ? bitmask : 0u);
            ssd1306->buffer[buffer_index + 2] = (ssd1306->buffer[buffer_index + 2] & inv_bitmask) | ((src_byte & (1u << 2)) ? bitmask : 0u);
            ssd1306->buffer[buffer_index + 3] = (ssd1306->buffer[buffer_index + 3] & inv_bitmask) | ((src_byte & (1u << 3)) ? bitmask : 0u);
            ssd1306->buffer[buffer_index + 4] = (ssd1306->buffer[buffer_index + 4] & inv_bitmask) | ((src_byte & (1u << 4)) ? bitmask : 0u);
            ssd1306->buffer[buffer_index + 5] = (ssd1306->buffer[buffer_index + 5] & inv_bitmask) | ((src_byte & (1u << 5)) ? bitmask : 0u);
            ssd1306->buffer[buffer_index + 6] = (ssd1306->buffer[buffer_index + 6] & inv_bitmask) | ((src_byte & (1u << 6)) ? bitmask : 0u);
            ssd1306->buffer[buffer_index + 7] = (ssd1306->buffer[buffer_index + 7] & inv_bitmask) | ((src_byte & (1u << 7)) ? bitmask : 0u);
        }

        // Handle the last partial byte when width is not aligned to 8 pixels.
        if (tail_bits != 0) {
            const uint16_t tail_byte_index = row_base + full_bytes;
            const uint8_t src_byte = bitmap_data[tail_byte_index];
            const uint16_t tail_buffer_base = buffer_row_base + (full_bytes << 3);

            for (uint16_t bit = 0; bit < tail_bits; bit++) {
                const uint16_t buffer_index = tail_buffer_base + bit;
                const uint8_t on_mask = (uint8_t)((src_byte & (1u << bit)) ? bitmask : 0u);
                ssd1306->buffer[buffer_index] = (ssd1306->buffer[buffer_index] & inv_bitmask) | on_mask;
            }
        }
    }

    return true;
}

bool ssd1306_draw_bitmap(ssd1306_t* ssd1306, const bitmap_t* bitmap, uint8_t start_x, uint8_t start_y) {
    if (!ssd1306_is_ready(ssd1306) || bitmap == NULL || bitmap->data == NULL) {
        return false;
    }
    return _ssd1306_draw_bitmap_internal(ssd1306, bitmap->data, 0, bitmap->width, bitmap->height, start_x, start_y);
}

void ssd1306_set_font(ssd1306_t* ssd1306, const font_t* font) {
    if (ssd1306 == NULL) {
        return;
    }
    ssd1306->font = font;
}

bool ssd1306_print(ssd1306_t* ssd1306, const char* text, uint8_t start_x, uint8_t start_y) {
    if (!ssd1306_is_ready(ssd1306)) {
        return false;
    }
    const font_t* font = ssd1306->font;
    if (font == NULL || text == NULL) {
        return false;
    }

    uint8_t current_x = start_x;
    uint16_t codepoint;

    while (*text != '\0') {
        if (current_x >= ssd1306->width) {
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
    if (!ssd1306_is_ready(ssd1306)) {
        return false;
    }

    const uint8_t pages = ssd1306->height / SSD1306_BITS_PER_COLUMN;

    if (!ssd1306_send_command_value(ssd1306, SSD1306_MEMORY_ADDRESSING_MODE_COMMAND, SSD1306_MEMORY_ADDRESSING_MODE_PAGE)) {
        return false;
    }

    uint8_t tx[1 + 128];
    tx[0] = SSD1306_SEND_DATA;

    for (uint8_t page = 0; page < pages; page++) {
        if (!ssd1306_send_command(ssd1306, (uint8_t)(0xB0 | page))) {
            return false;
        }
        if (!ssd1306_send_command(ssd1306, 0x00)) {
            return false;
        }
        if (!ssd1306_send_command(ssd1306, 0x10)) {
            return false;
        }

        memcpy(&tx[1], ssd1306->buffer + 1 + ((uint16_t)page * ssd1306->width), ssd1306->width);
        if (!i2c_write_exact(i2c_write_timeout_us(ssd1306->i2c_inst, ssd1306->i2c_address, tx, (size_t)ssd1306->width + 1, false, SSD1306_I2C_TIMEOUT_US), (size_t)ssd1306->width + 1)) {
            return false;
        }
    }

    return true;
}

void ssd1306_destroy(ssd1306_t *ssd1306) {
    if (ssd1306 == NULL || ssd1306->buffer == NULL) {
        return;
    }
    free(ssd1306->buffer);
    ssd1306->buffer = NULL;
    ssd1306->buffer_size = 0;
}

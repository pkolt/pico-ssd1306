/**
 * C Library for SSD1306 OLED Display
 * Author: Pavel Koltyshev
 * (c) 2025
*/

#ifndef SSD1306_DEF_H
#define SSD1306_DEF_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"
#include "bitmap.h"
#include "font.h"

#ifndef SSD1306_I2C_TIMEOUT_US
// Timeout for a full I2C transfer. Must be enough for framebuffer writes
// (e.g. 1025 bytes for 128x64), otherwise writes can be truncated.
#define SSD1306_I2C_TIMEOUT_US 100000
#endif

#define SSD1306_BITS_PER_COLUMN 8 // quantity segments in one column
#define SSD1306_BITS_IN_BYTE 8 // 1 byte = 8 bits

#define SSD1306_SEND_COMMAND 0x00
#define SSD1306_SEND_DATA 0x40

// 1. Fundamental Command
#define SSD1306_CONTRAST_COMMAND 0x81 // Value: 1-255
#define SSD1306_CONTRAST_DEFAULT 0x7F // 127

#define SSD1306_ENTIRE_DISPLAY_ON_COMMAND 0xA4 // Resume to RAM content display (RESET)
#define SSD1306_ENTIRE_DISPLAY_OFF_COMMAND 0xA5 // Entire display ON Output ignores RAM content

#define SSD1306_DISPLAY_NORMAL_COMMAND 0xA6 // Normal display (RESET)
#define SSD1306_DISPLAY_INVERSE_COMMAND 0xA7 // Inverse display

#define SSD1306_DISPLAY_OFF_COMMAND 0xAE // Display OFF (sleep mode) (RESET)
#define SSD1306_DISPLAY_ON_COMMAND 0xAF // Display ON in normal mode

// 2. Scrolling Command
// ...

// 3. Addressing Setting Command
// - Set Lower Column Start Address for Page Addressing Mode. This command is only for page addressing mode. 0x00~0x0F (0-15)
// - Set Higher Column Start Address for Page Addressing Mode. This command is only for page addressing mode. 0x10~0x1F (16-31)
// - Set Page Start Address for Page Addressing Mode. This command is only for page addressing mode. 0xB0~0xB7 (176-183)
#define SSD1306_MEMORY_ADDRESSING_MODE_COMMAND 0x20
typedef enum {
    SSD1306_MEMORY_ADDRESSING_MODE_HORIZONTAL = 0x00, // Horizontal Addressing Mode
    SSD1306_MEMORY_ADDRESSING_MODE_VERTICAL = 0x01, // Vertical Addressing Mode
    SSD1306_MEMORY_ADDRESSING_MODE_PAGE = 0x02 // Page Addressing Mode (RESET)
} ssd1306_memory_addressing_mode_t;

#define SSD1306_COLUMN_START_END_ADDRESS_COMMAND 0x21 // Set Column Address. This command is only for horizontal or vertical addressing mode. Setup column start and end address (0-127).
#define SSD1306_COLUMN_START_ADDRESS 0x00 // 0
#define SSD1306_COLUMN_END_ADDRESS 0x7F // 127

#define SSD1306_PAGE_START_END_ADDRESS_COMMAND 0x22 // Set Page start and end address (0-7). This command is only for horizontal or vertical addressing mode.
#define SSD1306_PAGE_START_ADDRESS 0x00 // 0
#define SSD1306_PAGE_END_ADDRESS 0x07 // 7

// 4. Hardware Configuration (Panel resolution & layout related) Command
// - Set Display Start Line. 0x40~0x7F (64-127)
#define SSD1306_DISPLAY_START_LINE_COMMAND 0x40 // Set Display First Line (0-63)

#define SSD1306_SEGMENT_RE_MAP_NORMAL_COMMAND 0xA0 // Set Segment Re-map. Column address 0 is mapped to SEG0 (RESET)
#define SSD1306_SEGMENT_RE_MAP_INVERSE_COMMAND 0xA1 // Set Segment Re-map. Column address 127 is mapped to SEG0

#define SSD1306_MUX_RATIO_COMMAND 0xA8 // Set Multiplex Ratio (15-63, N+1 MUX, 15=16MUX, 63=64MUX)
#define SSD1306_MUX_RATIO_MIN 0x0F // 15
#define SSD1306_MUX_RATIO_MAX 0x3F // 63 (RESET)

#define SSD1306_COM_OUTPUT_SCAN_DIRECTION_NORMAL_COMMAND 0xC0 // Set COM Output Scan Direction. Normal mode (RESET) Scan from COM0 to COM[N-1]
#define SSD1306_COM_OUTPUT_SCAN_DIRECTION_REMAPPED_COMMAND 0xC8 // Set COM Output Scan Direction. Remapped mode. Scan from COM[N-1] to COM0. Where N is the Multiplex ratio.

#define SSD1306_DISPLAY_OFFSET_COMMAND 0xD3 // Set Display Offset. Set vertical shift by COM from 0d~63d.
#define SSD1306_DISPLAY_OFFSET_MIN 0x00 // 0 (RESET)
#define SSD1306_DISPLAY_OFFSET_MAX 0x3F // 63

#define SSD1306_COM_PINS_HARDWARE_CONFIG_COMMAND 0xDA // Set COM Pins Hardware Configuration
#define SSD1306_COM_PINS_HARDWARE_CONFIG_SEQUENTIAL_COM_PIN 0x02 // Sequential COM pin configuration
#define SSD1306_COM_PINS_HARDWARE_CONFIG_ALTERNATIVE_COM_PIN 0x12 // Alternative COM pin configuration
#define SSD1306_COM_PINS_HARDWARE_CONFIG_DISABLE_REMAP 0x00 // Disable COM Left/Right remap (RESET)
#define SSD1306_COM_PINS_HARDWARE_CONFIG_ENABLE_REMAP 0x20 // Enable COM Left/Right remap


// 5. Timing & Driving Scheme Setting Command
#define SSD1306_DISPLAY_CLOCK_DIVIDE_COMMAND 0xD5 // Set Display Clock Divide Ratio/Oscillator Frequency.
#define SSD1306_DISPLAY_CLOCK_DIVIDE_RATIO_MIN 0x01 // 1 (RESET)
#define SSD1306_DISPLAY_CLOCK_DIVIDE_RATIO_MAX 0x0F // 15
#define SSD1306_DISPLAY_CLOCK_OSCILLATOR_FREQUENCY_MIN 0x00 // 0
#define SSD1306_DISPLAY_CLOCK_OSCILLATOR_FREQUENCY_MAX 0x0F // 15
#define SSD1306_DISPLAY_CLOCK_OSCILLATOR_FREQUENCY_DEFAULT 0x08 // 8 (RESET)

#define SSD1306_PRE_CHARGE_PERIOD_COMMAND 0xD9 // Set Pre-charge Period
#define SSD1306_PRE_CHARGE_PERIOD_PHASE_MIN 0x01 // 1
#define SSD1306_PRE_CHARGE_PERIOD_PHASE_MAX 0x0F // 15
#define SSD1306_PRE_CHARGE_PERIOD_PHASE1_DEFAULT 0x02 // (RESET)
#define SSD1306_PRE_CHARGE_PERIOD_PHASE2_DEFAULT 0x02 // (RESET)

#define SSD1306_VCOMH_DESELECT_LEVEL_COMMAND 0xDB // Set VCOMH Deselect Level
typedef enum {
    SSD1306_VCOMH_DESELECT_LEVEL0 = 0x00, // ~0.65 x VCC
    SSD1306_VCOMH_DESELECT_LEVEL1 = 0x20, // ~0.77 x VCC (RESET)
    SSD1306_VCOMH_DESELECT_LEVEL2 = 0x30 // ~0.83 x VCC
} ssd1306_vcomh_deselect_level_t;

// 6. Advance Graphic Command
#define SSD1306_FADE_OUT_BLINKING_COMMAND 0x23 // Set Fade, Out and Blinking.
typedef enum {
    SSD1306_FADE_OUT_BLINKING_DISABLE = 0x00, // Disable Fade Out / Blinking Mode (RESET)
    SSD1306_FADE_OUT_MODE = 0x20, // Enable Fade Out mode
    SSD1306_BLINKING_MODE = 0x30 // Enable Blinking mode
} ssd1306_fade_out_blinking_mode_t;
#define SSD1306_FADE_OUT_BLINKING_TIME_INTERVAL_MIN 0x00 // 0
#define SSD1306_FADE_OUT_BLINKING_TIME_INTERVAL_MAX 0x0F // 15

#define SSD1306_ZOOM_IN_COMMAND 0xD6 // Set Zoom In
#define SSD1306_ZOOM_IN_DISABLE 0x00 // Disable Zoom in Mode (RESET)
#define SSD1306_ZOOM_IN_ENABLE 0x01 // Enable Zoom in Mode

// 7. Charge Pump Command
#define SSD1306_CHARGE_PUMP_COMMAND 0x8D // Charge Pump Setting
#define SSD1306_CHARGE_PUMP_ENABLE 0x14 // Enable charge pump during display on
#define SSD1306_CHARGE_PUMP_DISABLE 0x10 // Disable charge pump (RESET)

typedef struct {
    // 1. Fundamental Command
    uint8_t contrast; // Value: 1-255
    bool inverse;

    // 2. Scrolling Command
    // ...

    // 3. Addressing Setting Command
    ssd1306_memory_addressing_mode_t memory_addressing_mode;

    // 4. Hardware Configuration Command
    bool segment_re_map_inverse;
    uint8_t mux_ratio; // Values: 15-63
    bool com_output_scan_direction_remapped;
    bool com_alt_pin_config;
    bool com_disable_left_right_remap;

    // 5. Timing & Driving Scheme Setting Command
    uint8_t divide_ratio; // Values: 1-15
    uint8_t oscillator_frequency; // Values: 0-15
    uint8_t pre_charge_period_phase_1; // Values: 1-15
    uint8_t pre_charge_period_phase_2; // Values: 1-15
    ssd1306_vcomh_deselect_level_t vcomh_deselect_level;

    // 6. Advance Graphic Command
    ssd1306_fade_out_blinking_mode_t fade_out_blinking_mode;
    uint8_t fade_out_time_interval; // Values: 0-15
    bool zoom;

    // 7. Charge Pump Command
    bool charge_pump;

} ssd1306_config_t;

typedef enum {
    SSD1306_DISPLAY_SIZE_128x64 = 0x00,
    SSD1306_DISPLAY_SIZE_128x32 = 0x01,
} ssd1306_display_size_t;

typedef struct {
    i2c_inst_t* i2c_inst;
    uint8_t i2c_address;
    uint8_t width;
    uint8_t height;
    const font_t* font;
    uint16_t buffer_size;
    uint8_t* buffer;
} ssd1306_t;

#endif // SSD1306_DEF_H

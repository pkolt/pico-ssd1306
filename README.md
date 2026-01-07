# Pico SSD1306

Library for working with OLED SSD1306 on Raspberry Pi Pico boards.

## Install

1. Install [Pico SDK](https://github.com/raspberrypi/pico-sdk) and set `PICO_SDK_PATH` (example: `/Users/bob/.pico-sdk/sdk/2.2.0`)
2. Install [Pico SSD1306](https://github.com/pkolt/pico-ssd1306) and set `PICO_SSD1306_PATH` (example: `/Users/bob/pico-ssd1306`)
3. Copy the file `external/pico_ssd1306_import.cmake` to the root of your project.
4. Add `include(pico_ssd1306_import.cmake)` to `CMakeLists.txt` after `include(pico_sdk_import.cmake)`.

## Use

```c
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "ssd1306.h"
#include "raspberry_pi_logo.h"
#include "google_sans_code_font.h"

#define SSD1306_I2C_ADDRESS 0x3C
#define I2C_PORT i2c0

#if defined(PICO_RP2040)
    #define I2C_SDA 4
    #define I2C_SCL 5
#elif defined(PICO_RP2350)
    #define I2C_SDA 20
    #define I2C_SCL 21
#else
    #error "Unsupported MCU: this code supports only RP2040 and RP2350"
#endif

int main() {
    stdio_init_all();

    // I2C Initialization. Using it at 400 kHz.
    i2c_init(I2C_PORT, 400*1000);
    
    // Setup I2C pins
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Create SSD1306
    ssd1306_t ssd1306 = ssd1306_create(I2C_PORT, SSD1306_I2C_ADDRESS);

    // Setup SSD1306
    ssd1306_config_t ssd1306_cfg = ssd1306_get_default_config();
    ssd1306_cfg.contrast = 100; // range 1-255

    if (!ssd1306_init(&ssd1306, &ssd1306_cfg)) {
        printf("Failed to initialize SSD1306\n");
        while (1) tight_loop_contents();
    }

    ssd1306_set_font(&ssd1306, &google_sans_code_font);

    while (true) {
        ssd1306_clear_display(&ssd1306);
        ssd1306_print(&ssd1306, "Hello world!", 0, 0);
        ssd1306_show(&ssd1306);
        sleep_ms(2000);

        ssd1306_clear_display(&ssd1306);
        ssd1306_draw_bitmap(&ssd1306, &raspberry_pi_logo, 0, 0);
        ssd1306_show(&ssd1306);
        sleep_ms(2000);
    }
}
```

## Make bitmap for SSD1306

[Create bitmap image](https://pkolt.github.io/bitmap_editor/)

[Convert font to bitmap](https://github.com/pkolt/font2bitmap)

Use LSB bit order for creating image.

## Build Library

```sh
cmake -S . -B build # Setup CMake build directory
cmake --build build --target pico_ssd1306 # Build library
```

## Build Example

```sh
cmake -S . -B build # OR cmake -S . -B build -DPICO_BOARD=pico2
cmake --build build --target ssd1306_example
```

## API

- `ssd1306_config_t ssd1306_get_default_config()` - Gets the default ssd1306 configuration.
- `ssd1306_t ssd1306_create(i2c_inst_t* i2c_inst, uint8_t i2c_address)` - Creates an ssd1306 instance.
- `bool ssd1306_init(ssd1306_t* ssd1306, const ssd1306_config_t* config)` - Initializes the ssd1306.
- `bool ssd1306_set_contrast(ssd1306_t* ssd1306, uint8_t contrast)` - Sets the contrast.
- `bool ssd1306_set_inverse(ssd1306_t* ssd1306, bool value)` - Sets inverse mode.
- `bool ssd1306_display_on(ssd1306_t* ssd1306)` - Turns on the display.
- `bool ssd1306_display_off(ssd1306_t* ssd1306)` - Turns off the display.
- `bool ssd1306_clear_display(ssd1306_t* ssd1306)` - Clears the display.
- `void ssd1306_set_font(ssd1306_t* ssd1306, const font_t* font)` - Sets the font.
- `bool ssd1306_print(ssd1306_t* ssd1306, const char* text, uint8_t start_x, uint8_t start_y)` - Prints text.
- `bool ssd1306_draw_bitmap(ssd1306_t* ssd1306, const bitmap_t* bitmap, uint8_t start_x, uint8_t start_y)` - Draws a bitmap.
- `bool ssd1306_show(ssd1306_t* ssd1306)` - Shows the display content.

## Compatibility

### MCU

- RP2040
- RP2350

### Displays

- 128x64

## Licenses

[MIT](./LICENSE.md)
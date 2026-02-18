# Pico SSD1306

Library for working with the SSD1306 OLED on Raspberry Pi Pico boards.

## Install

1. Install [Pico SDK](https://github.com/raspberrypi/pico-sdk) and set `PICO_SDK_PATH` (example: `/Users/<username>/.pico-sdk/sdk/2.2.0`)
2. Install [Pico SSD1306](https://github.com/pkolt/pico-ssd1306) and set `PICO_SSD1306_PATH` (example: `/Users/<username>/pico-ssd1306`)
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

// See the correct pins in the datasheet for your board
#define I2C_SDA 20
#define I2C_SCL 21

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

## Create a bitmap for SSD1306

[Create a bitmap image](https://pkolt.github.io/bitmap_editor/)

[Convert a font to bitmap](https://github.com/pkolt/font2bitmap)

Use LSB bit order when creating images.

## Build the Library

```sh
cmake -S . -B build # Set up CMake build directory
cmake --build build --target pico_ssd1306 # Build library
```

## Build Example

```sh
cmake -S . -B build # OR cmake -S . -B build -DPICO_BOARD=pico2
cmake --build build --target oled # See output files in `build/examples/oled`
```

## API

```c
// Gets the default ssd1306 configuration
ssd1306_config_t ssd1306_get_default_config()

// Creates an ssd1306 instance
ssd1306_t ssd1306_create(i2c_inst_t* i2c_inst, uint8_t i2c_address)

// Initializes the ssd1306
bool ssd1306_init(ssd1306_t* ssd1306, const ssd1306_config_t* config)

// Sets the contrast
bool ssd1306_set_contrast(ssd1306_t* ssd1306, uint8_t contrast)

// Sets inverse mode
bool ssd1306_set_inverse(ssd1306_t* ssd1306, bool value)

// Turns on the display
bool ssd1306_display_on(ssd1306_t* ssd1306)

// Turns off the display
bool ssd1306_display_off(ssd1306_t* ssd1306)

// Clears the display
bool ssd1306_clear_display(ssd1306_t* ssd1306)

// Sets the font
void ssd1306_set_font(ssd1306_t* ssd1306, const font_t* font)

// Prints text
bool ssd1306_print(ssd1306_t* ssd1306, const char* text, uint8_t start_x, uint8_t start_y)

// Draws a bitmap
bool ssd1306_draw_bitmap(ssd1306_t* ssd1306, const bitmap_t* bitmap, uint8_t start_x, uint8_t start_y)

// Shows the display content
bool ssd1306_show(ssd1306_t* ssd1306)
```

## Compatibility

### MCU

- RP2040
- RP2350

### Displays

- 128x64

## Set Up VS Code

Follow these steps so VS Code can find the Pico SDK headers (so types like `uint8_t` resolve):

1. Install the **C/C++** extension (`ms-vscode.cpptools`). Installing **CMake Tools** is recommended.
2. Ensure `PICO_SDK_PATH` and `PICO_SSD1306_PATH` are set in your shell or VS Code environment.
3. Generate `compile_commands.json` from CMake (this makes IntelliSense use the same include/flags as the build):

```bash
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

4. In VS Code, open the Command Palette -> **C/C++: Edit Configurations (UI)** and set:
	- **Compile commands** -> `${workspaceFolder}/build/compile_commands.json`
	- **Compiler path** -> your host compiler (for example `/usr/bin/clang` or `/usr/bin/gcc`) or to `arm-none-eabi-gcc` for Pico cross-compiles.

5. (Optional) In `.vscode/c_cpp_properties.json` add or verify the `compileCommands` entry:

```json
"compileCommands": "${workspaceFolder}/build/compile_commands.json"
```

6. After updating settings, run **C/C++: Reset IntelliSense Database** from the Command Palette or restart VS Code.

7. Troubleshooting: if `uint8_t` or `stdint.h` are still reported as missing, make sure the extension's `compilerPath` points to the same toolchain CMake used (for Pico cross-compiles point it to `arm-none-eabi-gcc`) so the extension picks up the correct sysroot/include paths.

Using the CMake Tools extension to configure the project will also generate `compile_commands.json` automatically when CMake config runs.

8. Reopen the project in VS Code.

## Licenses

[MIT](./LICENSE.md)

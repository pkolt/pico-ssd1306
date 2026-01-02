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
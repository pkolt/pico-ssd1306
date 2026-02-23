add_executable(oled_128x32
    main.c
    google_sans_code_24.h
    raspberry_pi_logo_128x32.h
)

target_link_libraries(oled_128x32
    pico_stdlib
    pico_ssd1306
)

pico_add_extra_outputs(oled_128x32)
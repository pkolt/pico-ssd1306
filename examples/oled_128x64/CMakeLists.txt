add_executable(oled_128x64
    main.c
    raspberry_pi_logo.h
    google_sans_code_32.h
)

target_link_libraries(oled_128x64
    pico_stdlib
    pico_ssd1306
)

pico_add_extra_outputs(oled_128x64)
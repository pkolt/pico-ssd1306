add_executable(oled
    main.c
    raspberry_pi_logo.h
    google_sans_code_font.h
)

target_link_libraries(oled
    pico_stdlib
    pico_ssd1306
)

pico_add_extra_outputs(oled)
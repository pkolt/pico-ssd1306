add_executable(ssd1306_example
    main.c
    raspberry_pi_logo.h
    google_sans_code_font.h
)

target_link_libraries(ssd1306_example
    pico_stdlib
    pico_ssd1306
)

pico_add_extra_outputs(ssd1306_example)
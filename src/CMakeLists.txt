add_library(pico_ssd1306
    ssd1306.c
)

target_include_directories(pico_ssd1306
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(pico_ssd1306
    pico_stdlib
    hardware_i2c
)

target_link_libraries(pico_ssd1306_included INTERFACE pico_ssd1306)

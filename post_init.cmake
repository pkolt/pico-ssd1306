add_library(pico_ssd1306_included INTERFACE)
target_compile_definitions(pico_ssd1306_included INTERFACE
        -DPICO_SSD1306=1
        )

pico_add_platform_library(pico_ssd1306_included)

# note as we're a .cmake included by the SDK, we're relative to the pico-sdk build
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src ${CMAKE_BINARY_DIR}/pico_ssd1306/src)

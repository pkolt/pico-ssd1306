cmake_minimum_required(VERSION 3.13)

if (NOT TARGET _pico_ssd1306_inclusion_marker)
    add_library(_pico_ssd1306_inclusion_marker INTERFACE)

    # Pull in PICO SDK (must be before project)
    include(pico_sdk_import.cmake)
    if (PICO_SDK_VERSION_STRING VERSION_LESS "2.0.0")
        message(FATAL_ERROR "Require at least Raspberry Pi Pico SDK version 2.0.0")
    endif()

    project(pico_ssd1306 C CXX)

    set(CMAKE_C_STANDARD 11)
    set(CMAKE_CXX_STANDARD 17)

    pico_is_top_level_project(PICO_SSD1306_TOP_LEVEL_PROJECT)

    if (NOT PICO_SSD1306_PATH)
        set(PICO_SSD1306_PATH ${CMAKE_CURRENT_LIST_DIR})
    endif()
    set(PICO_SSD1306_PATH "${PICO_SSD1306_PATH}" CACHE PATH "Path to Pico SSD1306")

    # The real work gets done in post_init which is called at the end of pico_sdk_init
    list(APPEND PICO_SDK_POST_LIST_FILES ${CMAKE_CURRENT_LIST_DIR}/post_init.cmake)
    if (PICO_SSD1306_TOP_LEVEL_PROJECT)
        message("pico_ssd1306: initialize SDK since we're the top-level")
        # Initialize the SDK
        pico_sdk_init()
    else()
        set(PICO_SDK_POST_LIST_FILES ${PICO_SDK_POST_LIST_FILES} PARENT_SCOPE)
    endif()

    if(PICO_SSD1306_TOP_LEVEL_PROJECT)
        add_subdirectory(examples)
    endif()
endif()
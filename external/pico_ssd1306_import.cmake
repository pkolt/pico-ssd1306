# This is a copy of <PICO_SSD1306_PATH>/external/pico_ssd1306_import.cmake

# This can be dropped into an external project to help locate pico-ssd1306
# It should be include()ed prior to project()

if (DEFINED ENV{PICO_SSD1306_PATH} AND (NOT PICO_SSD1306_PATH))
    set(PICO_SSD1306_PATH $ENV{PICO_SSD1306_PATH})
    message("Using PICO_SSD1306_PATH from environment ('${PICO_SSD1306_PATH}')")
endif ()

if (DEFINED ENV{PICO_SSD1306_FETCH_FROM_GIT} AND (NOT PICO_SSD1306_FETCH_FROM_GIT))
    set(PICO_SSD1306_FETCH_FROM_GIT $ENV{PICO_SSD1306_FETCH_FROM_GIT})
    message("Using PICO_SSD1306_FETCH_FROM_GIT from environment ('${PICO_SSD1306_FETCH_FROM_GIT}')")
endif ()

if (DEFINED ENV{PICO_SSD1306_FETCH_FROM_GIT_PATH} AND (NOT PICO_SSD1306_FETCH_FROM_GIT_PATH))
    set(PICO_SSD1306_FETCH_FROM_GIT_PATH $ENV{PICO_SSD1306_FETCH_FROM_GIT_PATH})
    message("Using PICO_SSD1306_FETCH_FROM_GIT_PATH from environment ('${PICO_SSD1306_FETCH_FROM_GIT_PATH}')")
endif ()

if (NOT PICO_SSD1306_PATH)
    if (PICO_SSD1306_FETCH_FROM_GIT)
        include(FetchContent)
        set(FETCHCONTENT_BASE_DIR_SAVE ${FETCHCONTENT_BASE_DIR})
        if (PICO_SSD1306_FETCH_FROM_GIT_PATH)
            get_filename_component(FETCHCONTENT_BASE_DIR "${PICO_SSD1306_FETCH_FROM_GIT_PATH}" REALPATH BASE_DIR "${CMAKE_SOURCE_DIR}")
        endif ()
        FetchContent_Declare(
                pico_ssd1306
                GIT_REPOSITORY https://github.com/pkolt/pico-ssd1306
                GIT_TAG master
        )
        if (NOT pico_ssd1306)
            message("Downloading Raspberry Pi Pico Extras")
            FetchContent_Populate(pico_ssd1306)
            set(PICO_SSD1306_PATH ${pico_ssd1306_SOURCE_DIR})
        endif ()
        set(FETCHCONTENT_BASE_DIR ${FETCHCONTENT_BASE_DIR_SAVE})
    else ()
        if (PICO_SDK_PATH AND EXISTS "${PICO_SDK_PATH}/../pico-ssd1306")
            set(PICO_SSD1306_PATH ${PICO_SDK_PATH}/../pico-ssd1306)
            message("Defaulting PICO_SSD1306_PATH as sibling of PICO_SDK_PATH: ${PICO_SSD1306_PATH}")
        else()
            message(FATAL_ERROR
                    "PICO EXTRAS location was not specified. Please set PICO_SSD1306_PATH or set PICO_SSD1306_FETCH_FROM_GIT to on to fetch from git."
                    )
        endif()
    endif ()
endif ()

set(PICO_SSD1306_PATH "${PICO_SSD1306_PATH}" CACHE PATH "Path to the PICO EXTRAS")
set(PICO_SSD1306_FETCH_FROM_GIT "${PICO_SSD1306_FETCH_FROM_GIT}" CACHE BOOL "Set to ON to fetch copy of PICO EXTRAS from git if not otherwise locatable")
set(PICO_SSD1306_FETCH_FROM_GIT_PATH "${PICO_SSD1306_FETCH_FROM_GIT_PATH}" CACHE FILEPATH "location to download EXTRAS")

get_filename_component(PICO_SSD1306_PATH "${PICO_SSD1306_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
if (NOT EXISTS ${PICO_SSD1306_PATH})
    message(FATAL_ERROR "Directory '${PICO_SSD1306_PATH}' not found")
endif ()

set(PICO_SSD1306_PATH ${PICO_SSD1306_PATH} CACHE PATH "Path to the PICO EXTRAS" FORCE)

add_subdirectory(${PICO_SSD1306_PATH} pico_ssd1306)
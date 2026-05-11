cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico_plc_controller C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

pico_sdk_init()

add_library(plc_board
    src/plc_board.cpp
    src/ssd1306.cpp
    src/ws2812_status.cpp
)

pico_generate_pio_header(plc_board ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio)

target_include_directories(plc_board
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(plc_board
    PUBLIC
        pico_stdlib
        hardware_i2c
        hardware_clocks
        hardware_pio
)

add_executable(controller_demo
    demo/controller_demo.cpp
)

target_link_libraries(controller_demo
    PRIVATE
        plc_board
)

pico_enable_stdio_usb(controller_demo 1)
pico_enable_stdio_uart(controller_demo 0)
pico_add_extra_outputs(controller_demo)

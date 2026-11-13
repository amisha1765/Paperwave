
cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2 CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(PaperWave_SD_Test C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(PaperWave_SD_Test
    main.c
    sd_card.c
)

target_link_libraries(PaperWave_SD_Test
    pico_stdlib
    hardware_spi
    hardware_gpio
)

pico_enable_stdio_usb(PaperWave_SD_Test 1)
pico_enable_stdio_uart(PaperWave_SD_Test 0)
pico_add_extra_outputs(PaperWave_SD_Test)

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)


project(Frackstock C CXX ASM)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)
set(PICO_CXX_ENABLE_RTTI 1)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

include("lib/PicoLed/PicoLed.cmake")
include_directories("include")

# rest of your project
add_executable(Frackstock
    source/main.cpp
    source/led_ring.cpp
    source/device.cpp
    source/frackstock.cpp
    source/radio.cpp
    source/cc1101.cpp
    source/serial.cpp
)

pico_set_program_name(Frackstock "Frackstock")
pico_set_program_version(Frackstock "0.4")

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(
    Frackstock 
    pico_stdlib 
    hardware_flash
    hardware_i2c 
    hardware_spi
    hardware_uart
    PicoLed
)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(Frackstock)

# enable usb output, disable uart output
pico_enable_stdio_usb(Frackstock 1)
pico_enable_stdio_uart(Frackstock 0)


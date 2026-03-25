# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

set(PICO_EXTRAS_FETCH_FROM_GIT ON)
include(pico_extras_import.cmake)
message("PICO_EXTRAS_PATH: ${PICO_EXTRAS_PATH}")

project(pico_8ball C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(pico_8ball 
         src/main.c
         src/GC9A01.c
         src/image.c
         src/battery.c
         src/MPU9250.c
         src/phymodel.c
         src/prophet.c
         src/font24eng.c
         src/font24cyr.c
         src/prophet_ua.c
         src/prophet_en.c)

pico_set_program_name(pico_8ball "pico_8ball")
pico_set_program_version(pico_8ball "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(pico_8ball 1)
pico_enable_stdio_usb(pico_8ball 0)

# Add the standard library to the build
target_link_libraries(pico_8ball
        pico_stdlib 
        hardware_spi 
        hardware_dma 
        hardware_adc 
        hardware_i2c 
        hardware_pwm 
        pico_multicore 
        hardware_pio
        
        hardware_clocks
        hardware_rosc
        hardware_sleep    
        hardware_pll
      
        
        )

# Add the standard include files to the build
target_include_directories(pico_8ball PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
		${PICO_EXTRAS_PATH}/src/rp2_common/pico_sleep/include 
		${PICO_EXTRAS_PATH}/src/rp2_common/hardware_rtc/include 
        ${PICO_EXTRAS_PATH}/src/rp2_common/hardware_rosc/include
        ${PICO_SDK_PATH}/src/rp2_common/pico_aon_timer/include
        ${PICO_SDK_PATH}/src/rp2_common/pico_aon_timer/include
        ${PICO_SDK_PATH}/src/rp2_common/hardware_powman/include
        
)

pico_add_extra_outputs(pico_8ball)


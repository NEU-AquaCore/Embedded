cmake_minimum_required(VERSION 3.20)

project(STM32_LED_Blink LANGUAGES C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    startup/startup.s
    src/main.c
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -mcpu=cortex-m3
    -mthumb
    -ffunction-sections
    -fdata-sections
    -Wall
    -Wextra
    -O0
    -g3
)

target_link_options(${PROJECT_NAME} PRIVATE
    -mcpu=cortex-m3
    -mthumb
    -T${CMAKE_SOURCE_DIR}/linker/stm32f103c8.ld
    -Wl,--gc-sections
    -Wl,-Map=${PROJECT_NAME}.map
    -Wl,--print-memory-usage
    -nostartfiles
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    SUFFIX ".elf"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
        -O ihex
        $<TARGET_FILE:${PROJECT_NAME}>
        ${PROJECT_NAME}.hex

    COMMAND ${CMAKE_OBJCOPY}
        -O binary
        $<TARGET_FILE:${PROJECT_NAME}>
        ${PROJECT_NAME}.bin

    COMMAND ${CMAKE_SIZE}
        $<TARGET_FILE:${PROJECT_NAME}>
)
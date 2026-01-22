cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PROJECT "emalloc-rp2040-profile")

set(PICO_PLATFORM rp2040)
set(PICO_BOARD pico)

set(FREERTOS_KERNEL_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/components/FreeRTOS/FreeRTOS/)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include(
  ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake
)

project(${PROJECT} C CXX ASM)

pico_sdk_init()

add_executable(
  ${PROJECT}
  src/main.cpp src/cycles.cpp src/alloc_wrappers.cpp src/tests.cpp
  ${CMAKE_CURRENT_LIST_DIR}/components/emalloc/emalloc/emalloc/src/emalloc.c)

target_link_libraries(${PROJECT} pico_stdlib pico_malloc FreeRTOS-Kernel-Heap4)

target_include_directories(
  ${PROJECT}
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/components/FreeRTOS/
          ${CMAKE_CURRENT_LIST_DIR}/components/emalloc/emalloc/emalloc/include/)

target_compile_definitions(${PROJECT} PRIVATE PICO_HEAP_SIZE=65536
                                              PICO_MALLOC_PANIC=0)

pico_add_extra_outputs(${PROJECT})

pico_enable_stdio_usb(${PROJECT} 0)
pico_enable_stdio_uart(${PROJECT} 1)

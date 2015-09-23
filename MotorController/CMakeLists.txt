set(CMAKE_TOOLCHAIN_FILE cmake/ArduinoToolchain.cmake) # Arduino Toolchain


cmake_minimum_required(VERSION 2.8)
#====================================================================#
#  Setup Project                                                     #
#====================================================================#
project(MotorController C CXX ASM)

#SET(ASM_OPTIONS "-x assembler-with-cpp")
#SET(CMAKE_ASM_FLAGS "${CFLAGS} ${ASM_OPTIONS}" )
SET(CMAKE_C_FLAGS "${CFLAGS} -std=gnu99 -O2" ) #-O2 optimization, otherwise delay.h complains

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Wire)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Wire/utility)

print_board_list()
print_programmer_list()

file(GLOB C_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS src/*.cpp src/*.c Wire/*.cpp Wire/utility/*.c)

generate_arduino_firmware(motor_controller
SRCS ${C_SRC}
BOARD mini328)

#add_custom_command(TARGET motor_controller
#                   POST_BUILD
#                   COMMAND avr-objcopy -O ihex input_controller.elf input_controller.hex)

set(${FIRMWARE_NAME}_PORT /dev/ttyUSB0)

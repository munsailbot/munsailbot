cmake_minimum_required(VERSION 2.8)

find_program(CMAKE_C_COMPILER NAMES arm-linux-gnueabihf-gcc-4.7)
find_program(CMAKE_CXX_COMPILER NAMES arm-linux-gnueabihf-g++-4.7)
find_program(CMAKE_AR NAMES arm-linux-gnueabihf-ar-4.7)
find_program(CMAKE_LINKER NAMES arm-linux-gnueabihf-g++-4.7)

message("USING CXX COMPILER: ${CMAKE_CXX_COMPILER}")
message("USING CXX FLAGS: ${CMAKE_CXX_FLAGS}")

project(BeagleCode)



add_subdirectory(SailbotBrain)
add_subdirectory(BeagleUtil)
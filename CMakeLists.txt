cmake_minimum_required(VERSION 3.5)

project(jack-corruptor VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND options
        -Wall -Wextra -Wconversion -Wsign-conversion -Wtype-limits
        -Wmissing-declarations
        -Wno-unused-function -Wno-unused-parameter
        -Werror=return-type
    )
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        list(APPEND options
            -Wmissing-variable-declarations
        )
    endif()
endif ()

## Add dependencies

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules")

find_package(Jack REQUIRED)

## Application


add_executable(jack-corruptor
    jack-corruptor.c
)

target_compile_options(jack-corruptor PRIVATE "${options}")
target_link_libraries(jack-corruptor PRIVATE
    jack
)

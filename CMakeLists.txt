cmake_minimum_required(VERSION 3.16)
project(median_filter_task LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_library(device_model src/device.cpp src/sobel_filter.cpp src/median_histogram.cpp src/host.cpp)
target_include_directories(device_model PUBLIC include src)
target_link_libraries(device_model PUBLIC Threads::Threads)

add_library(median_reference src/median_reference.c)
target_include_directories(median_reference PUBLIC include)

add_library(y4m_io src/y4m.cpp)
target_include_directories(y4m_io PUBLIC include)

add_library(sobel_reference src/sobel_reference.c)
target_include_directories(sobel_reference PUBLIC include)
if(UNIX)
    target_link_libraries(sobel_reference PUBLIC m)
endif()

add_executable(median_demo src/main.cpp)
target_link_libraries(median_demo PRIVATE device_model median_reference y4m_io)

add_executable(sobel_reference_demo src/sobel_reference_main.cpp)
target_link_libraries(sobel_reference_demo PRIVATE device_model sobel_reference y4m_io)

add_executable(median_tests tests/median_tests.cpp)
target_link_libraries(median_tests PRIVATE device_model median_reference)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/data")
configure_file(data/lena_mono.y4m data/lena_mono.y4m COPYONLY)

foreach(target device_model median_reference y4m_io median_demo sobel_reference sobel_reference_demo median_tests)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()

cmake_minimum_required(VERSION 2.8)
project(stereoGuidedFilter)

find_package(PNG)

if(NOT PNG_FOUND)
    find_package(ZLIB)
    if(NOT ZLIB_FOUND)
        add_subdirectory(third_party/zlib-1.2.8)
    endif(NOT ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    add_subdirectory(third_party/libpng-1.6.12)
endif(NOT PNG_FOUND)

add_definitions(${PNG_DEFINITIONS})
include_directories(${PNG_INCLUDE_DIRS})

set(SRC_C
    io_png.c io_png.h)

set(SRC
    cmdLine.h
    costVolume.cpp costVolume.h
    filters.cpp
    image.cpp image.h
    main.cpp
    occlusion.cpp occlusion.h)

add_executable(stereoGuidedFilter ${SRC} ${SRC_C})
target_link_libraries(stereoGuidedFilter ${PNG_LIBRARIES})

add_executable(show_weights
  cmdLine.h filters.cpp image.cpp image.h main_weights.cpp ${SRC_C})
target_link_libraries(show_weights ${PNG_LIBRARIES})

find_package(OpenMP)
if(OPENMP_FOUND)
    set_target_properties(stereoGuidedFilter PROPERTIES
                          COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        set(CMAKE_EXE_LINKER_FLAGS ${OpenMP_CXX_FLAGS})
    endif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
endif(OPENMP_FOUND)

if(UNIX)
    set_source_files_properties(${SRC} PROPERTIES
                                COMPILE_FLAGS "-Wall -Wextra -Werror -std=c++98")
    set_source_files_properties(${SRC_C} PROPERTIES
                                COMPILE_FLAGS "-Wall -Wextra -Werror -std=c89")
endif(UNIX)
if(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
endif(MSVC)

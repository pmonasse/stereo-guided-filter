CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

FIND_PACKAGE(PNG)

IF(NOT PNG_FOUND)
    FIND_PACKAGE(ZLIB)
    IF(NOT ZLIB_FOUND)
        ADD_SUBDIRECTORY(third_party/zlib-1.2.8)
    ENDIF(NOT ZLIB_FOUND)
    INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})
    ADD_SUBDIRECTORY(third_party/libpng-1.6.12)
    INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIRS})
ENDIF(NOT PNG_FOUND)

SET(SRC_C
    io_png.c io_png.h)

SET(SRC
    cmdLine.h
    costVolume.cpp costVolume.h
    filters.cpp
    image.cpp image.h
    main.cpp
    occlusion.cpp occlusion.h)

ADD_EXECUTABLE(costVolumeFilter ${SRC} ${SRC_C})
TARGET_LINK_LIBRARIES(costVolumeFilter ${PNG_LIBRARIES})

ADD_EXECUTABLE(show_weights
  cmdLine.h filters.cpp image.cpp image.h main_weights.cpp ${SRC_C})
TARGET_LINK_LIBRARIES(show_weights ${PNG_LIBRARIES})

FIND_PACKAGE(OpenMP)
IF(OPENMP_FOUND)
    SET_TARGET_PROPERTIES(costVolumeFilter PROPERTIES
                          COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
ENDIF(OPENMP_FOUND)

IF(UNIX)
    SET_SOURCE_FILES_PROPERTIES(${SRC} PROPERTIES
                                COMPILE_FLAGS "-Wall -Wextra -Werror -std=c++98 ${OpenMP_CXX_FLAGS}")
    SET_SOURCE_FILES_PROPERTIES(${SRC_C} PROPERTIES
                                COMPILE_FLAGS "-Wall -Wextra -Werror -std=c89")
    IF(OPENMP_FOUND)
        SET(CMAKE_EXE_LINKER_FLAGS ${OpenMP_CXX_FLAGS})
    ENDIF(OPENMP_FOUND)
ENDIF(UNIX)

SET(DATA
    data/tsukuba0.png data/tsukuba1.png
    data/disparity.png
    data/disparity_occlusion.png
    data/disparity_occlusion_filled.png
    data/disparity_occlusion_filled_smoothed.png)

SET(CPACK_PACKAGE_FILE_NAME "CostVolumeFilter-1.0")
SET(CPACK_GENERATOR "TGZ")
INCLUDE(CPack)
INSTALL(FILES CMakeLists.txt ${SRC} ${SRC_C} README.txt license.txt
        DESTINATION .)
INSTALL(FILES ${DATA}
        DESTINATION ./data)

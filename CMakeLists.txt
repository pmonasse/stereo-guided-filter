CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

FIND_PACKAGE(PNG)

SET(SRC
    cmdLine.h
    costVolume.cpp costVolume.h
    filters.cpp
    image.cpp image.h
    io_png.c io_png.h
    main.cpp
    occlusion.cpp occlusion.h)

ADD_EXECUTABLE(costVolumeFilter ${SRC})
TARGET_LINK_LIBRARIES(costVolumeFilter ${PNG_LIBRARIES})

FIND_PACKAGE(OpenMP)
IF(OPENMP_FOUND)
    SET_TARGET_PROPERTIES(costVolumeFilter PROPERTIES
                          COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
ENDIF(OPENMP_FOUND)

IF(UNIX)
    SET_TARGET_PROPERTIES(costVolumeFilter PROPERTIES
                          COMPILE_FLAGS "-Wall -Wextra -Werror ${OpenMP_CXX_FLAGS}")
    IF(OPENMP_FOUND)
        SET(CMAKE_EXE_LINKER_FLAGS  ${OpenMP_CXX_FLAGS})
    ENDIF(OPENMP_FOUND)
ENDIF(UNIX)

SET(DATA
    data/tsukuba0.png data/tsukuba1.png
    data/disparity.png
    data/disparity_occlusion.png
    data/disparity_occlusion_filled.png
    data/disparity_occlusion_filled_smoothed.png)

SET(CPACK_PACKAGE_FILE_NAME "CostVolumeFilter")
SET(CPACK_GENERATOR "TGZ")
INCLUDE(CPack)
INSTALL(FILES CMakeLists.txt ${SRC} README.txt license.txt
        DESTINATION .)
INSTALL(FILES ${DATA}
        DESTINATION ./data)

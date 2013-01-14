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

SET(CPACK_PACKAGE_FILE_NAME "CostVolumeFilter")
SET(CPACK_GENERATOR "TGZ")
INCLUDE(CPack)
INSTALL(FILES CMakeLists.txt ${SRC} README.txt tsukuba0.png tsukuba1.png
        DESTINATION .)
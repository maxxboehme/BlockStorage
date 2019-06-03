
ExternalProject_Add(
    DrMemory
    # DEPENDS
    PREFIX            ${DEPENDENCY_PREFIX}
    #--Download step--------------
    URL               https://github.com/DynamoRIO/drmemory/releases/download/release_1.11.0/DrMemory-Linux-1.11.0-2.tar.gz
    URL_HASH          SHA1=bdcaa95a117ae69a7fcf81094fe41f68f6d0189f
    #--Update/Patch step----------
    UPDATE_COMMAND    ""
    PATCH_COMMAND     ""
    CONFIGURE_COMMAND ""
    #--Configure step-------------
    SOURCE_DIR        ${CMAKE_CURRENT_BINARY_DIR}/DrMemory
    #    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX:PATH=${DEPENDENCY_INSTALL_PREFIX}
    #                      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    #--Build step-----------------
    #    BINARY_DIR        ${CMAKE_BINARY_DIR}/DrMemory
    BUILD_COMMAND     ""
    #--Install step----------------
    INSTALL_COMMAND   ""
    INSTALL_DIR       ""
)

ExternalProject_Get_Property(DrMemory source_dir)
set(DRMEMORY_COMMAND ${source_dir}/bin/drmemory CACHE INTERNAL "")

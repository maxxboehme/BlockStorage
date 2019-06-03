
ExternalProject_Add(
    Catch
    # DEPENDS
    PREFIX            ${DEPENDENCY_PREFIX}
    #--Download step--------------
    GIT_REPOSITORY    https://www.github.com/philsquared/Catch.git
    GIT_TAG           v1.7.0
    #--Update/Patch step----------
    UPDATE_COMMAND    ""
    #--Configure step-------------
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX:PATH=${DEPENDENCY_INSTALL_PREFIX}
                      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    #--Build step-----------------
    BUILD_COMMAND     ${CMAKE_COMMAND} --build .
    #--Install step----------------
    INSTALL_DIR       ${DEPENDENCY_INSTALL_PREFIX}
)

ExternalProject_Get_Property(Catch install_dir)
set(CATCH_INCLUDE_DIR ${install_dir}/include/catch CACHE INTERNAL "Path to include folder for Catch")

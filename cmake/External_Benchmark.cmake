
ExternalProject_Add(
    Benchmark
    # DEPENDS
    PREFIX            ${DEPENDENCY_PREFIX}
    #--Download step--------------
    GIT_REPOSITORY    https://www.github.com/google/benchmark.git
    GIT_TAG           v1.4.0
    #--Update/Patch step----------
    UPDATE_COMMAND    ""
    #--Configure step-------------
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX:PATH=${DEPENDENCY_INSTALL_PREFIX}
                      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                      -DBENCHMARK_DOWNLOAD_DEPENDENCIES=ON
    #--Build step-----------------
    BUILD_COMMAND     ${CMAKE_COMMAND} --build .
    #--Install step----------------
    INSTALL_DIR       ${DEPENDENCY_INSTALL_PREFIX}
)

ExternalProject_Get_Property(Benchmark install_dir)
set(BENCHMARK_INCLUDE_DIR ${install_dir}/include CACHE INTERNAL "Path to include folder for Benchmark")

set(LIB_SUFFIX "${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(LIB_PREFIX "${CMAKE_STATIC_LIBRARY_PREFIX}")

set(BENCHMARK_LIBRARY ${install_dir}/lib/${LIB_PREFIX}benchmark${LIB_SUFFIX} CACHE INTERNAL "Path to library folder for Benchmark")

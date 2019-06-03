cmake_minimum_required(VERSION 3.2)
cmake_policy(SET CMP0048 NEW)
# The following line suppresses warning about adding a dependency when a target does not exist.
# This usually happens when we have found locally an external dependency rather than having to download and
# build it during the build process.
cmake_policy(SET CMP0046 OLD)
message(STATUS "CMake version: ${CMAKE_VERSION}")

set(BLOCKSTORAGE_VERSION_MAJOR 0)
set(BLOCKSTORAGE_VERSION_MINOR 1)
set(BLOCKSTORAGE_VERSION_PATCH 0)
set(BLOCKSTORAGE_VERSION ${BLOCKSTORAGE_VERSION_MAJOR}.${BLOCKSTORAGE_VERSION_MINOR}.${BLOCKSTORAGE_VERSION_PATCH})
# project(BlockStorage VERSION ${BLOCKSTORAGE_VERSION} LANGUAGES C CXX)
project(BlockStorage C CXX)


set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
   ${CMAKE_MODULE_PATH})


# Setting up standard defaults, these will be passed down into external projects
# include(GenerateExportHeader)
include(BuildType)
include(download_dir)
include(ExternalProjectUtils)

# Add the third party dependencies
find_package(Threads REQUIRED)
find_package_external(PACKAGE Catch REQUIRE)

# Collecting header and source files for BlockStorage
file(GLOB BLOCKSTORAGE_HEADERS "src/*.h")
file(GLOB BLOCKSTORAGE_SOURCES "src/*.cpp")

add_executable(BlockStorage ${BLOCKSTORAGE_HEADERS} ${BLOCKSTORAGE_SOURCES})
add_dependency_external(TARGET BlockStorage PACKAGE Catch)
target_include_directories(BlockStorage PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(BlockStorage PRIVATE ${CATCH_INCLUDE_DIR})
target_link_libraries(BlockStorage ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
target_compile_features(BlockStorage PRIVATE cxx_nullptr)
target_compile_features(BlockStorage PUBLIC cxx_rvalue_references cxx_noexcept cxx_variadic_templates cxx_strong_enums cxx_generic_lambdas)

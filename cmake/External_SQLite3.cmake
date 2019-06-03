file(WRITE ${CMAKE_BINARY_DIR}/temp/CMakeLists.txt
   "cmake_minimum_required(VERSION 3.2)\n"
   "project(sqlite3 C)\n"
   "\n"
   "find_package(Threads REQUIRED)\n"
   "\n"
   "add_library(sqlite3 STATIC sqlite3.c sqlite3.h sqlite3ext.h)\n"
   "target_link_libraries(sqlite3 \${CMAKE_THREAD_LIBS_INIT} \${CMAKE_DL_LIBS})\n"
   "set_target_properties(sqlite3 PROPERTIES OUTPUT_NAME sqlite3)\n"
   "\n"
   "add_executable(sqlite sqlite3.c shell.c sqlite3.h sqlite3ext.h)\n"
   "target_link_libraries(sqlite \${CMAKE_THREAD_LIBS_INIT} \${CMAKE_DL_LIBS})\n"
   "\n"
   "add_definitions(-DSQLITE_ENABLE_RTREE)\n"
   "add_definitions(-DSQLITE_ENABLE_JSON1)\n"
   "install(TARGETS sqlite DESTINATION bin)\n"
   "install(TARGETS sqlite3 DESTINATION lib)\n"
   "install(FILES sqlite3.h DESTINATION include)\n"
)

ExternalProject_Add(
    SQLite3
    # DEPENDS
    PREFIX            ${DEPENDENCY_PREFIX}
    #--Download step--------------
    URL               http://www.sqlite.org/2017/sqlite-autoconf-3160200.tar.gz
    URL_HASH          SHA1=64ca578ad44a94115b1db0406740e14288f74bb8
    #--Update/Patch step----------
    UPDATE_COMMAND    ${CMAKE_COMMAND} -E copy
                           ${CMAKE_BINARY_DIR}/temp/CMakeLists.txt
                           ${DEPENDENCY_PREFIX}/src/SQLite3/CMakeLists.txt
    #--Configure step-------------
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX:PATH=${DEPENDENCY_INSTALL_PREFIX}
                      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    #--Build step-----------------
    BUILD_COMMAND     ${CMAKE_COMMAND} --build .
    #--Install step----------------
    INSTALL_DIR       ${DEPENDENCY_INSTALL_PREFIX}
)

ExternalProject_Get_Property(SQLite3 install_dir)
ExternalProject_Get_Property(SQLite3 binary_dir)
set(SQLITE3_INCLUDE_DIR ${install_dir}/include CACHE INTERNAL "")
set(SQLITE3_LIBRARIES_DIR ${install_dir}/lib)

if(WIN32)
    set(prefix "")
    set(suffix ".lib")
elseif(APPLE)
    set(prefix "lib")
    set(suffix ".a")
else()
    set(prefix "lib")
    set(suffix ".a")
endif()

set(SQLITE3_LIBRARY "${SQLITE3_LIBRARIES_DIR}/${prefix}sqlite3${suffix}" CACHE INTERNAL "")

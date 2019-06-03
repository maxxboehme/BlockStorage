# This script defines a download_dir variable, if it's not already defined.
if(NOT DEFINED download_dir)
   set(download_dir "${CMAKE_BINARY_DIR}/downloads")
endif()

file(MAKE_DIRECTORY "${download_dir}")
if(NOT EXISTS "${download_dir}")
  message(FATAL_ERROR "could not find or make Downloads directory")
endif()

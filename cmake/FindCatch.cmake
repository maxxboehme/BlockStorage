# - Find Catch
# Find the native Catch headers and libraries.
#
# CATCH_INCLUDE_DIRS - where to find sqlite3.h, etc.
# CATCH_LIBRARIES    - List of libraries when using sqlite.
# CATCH_FOUND        - True if sqlite found.

# Look for the header file.
FIND_PATH(CATCH_INCLUDE_DIR NAMES catch.hpp)

# Catch is header only and does not have a library.
SET(CATCH_LIBRARY "")

# Handle the QUIETLY and REQUIRED arguments and set CATCH_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CATCH DEFAULT_MSG CATCH_INCLUDE_DIR)

# Copy the results to the output variables.
IF(CATCH_FOUND)
    SET(CATCH_LIBRARIES ${CATCH_LIBRARY})
    SET(CATCH_INCLUDE_DIRS ${CATCH_INCLUDE_DIR})
ELSE(CATCH_FOUND)
    SET(CATCH_LIBRARIES)
    SET(CATCH_INCLUDE_DIRS)
ENDIF(CATCH_FOUND)

MARK_AS_ADVANCED(CATCH_INCLUDE_DIRS CATCH_LIBRARIES)


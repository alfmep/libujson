# Allow user to set GMP_ROOT_DIR to a custom GMP installation directory
#
set (GMP_ROOT_HINTS ${GMP_ROOT_DIR} ENV GMP_ROOT_DIR)

# Set GMP_INCLUDE_DIRS
#
find_path (GMP_INCLUDE_DIR NAMES gmp.h ${GMP_ROOT_HINTS})
set (GMP_INCLUDE_DIRS ${GMP_INCLUDE_DIR})

# Set GMP_VERSION
#
if ("${GMP_INCLUDE_DIR}" STREQUAL "GMP_INCLUDE_DIR-NOTFOUND")
    message (STATUS "Could not find include file gmp.h")
else()
    file (STRINGS "${GMP_INCLUDE_DIR}/gmp.h" __GNU_MP_VERSION REGEX "^#define[ \t]+__GNU_MP_VERSION[ \t]+[0-9]+.*")
    file (STRINGS "${GMP_INCLUDE_DIR}/gmp.h" __GNU_MP_VERSION_MINOR REGEX "^#define[ \t]+__GNU_MP_VERSION_MINOR[ \t]+[0-9]+.*")
    file (STRINGS "${GMP_INCLUDE_DIR}/gmp.h" __GNU_MP_VERSION_PATCHLEVEL REGEX "^#define[ \t]+__GNU_MP_VERSION_PATCHLEVEL[ \t]+[0-9]+.*")
    string (REGEX MATCH "[0-9]+" GMP_VERSION_MAJOR "${__GNU_MP_VERSION}")
    string (REGEX MATCH "[0-9]+" GMP_VERSION_MINOR "${__GNU_MP_VERSION_MINOR}")
    string (REGEX MATCH "[0-9]+" GMP_VERSION_PATCH "${__GNU_MP_VERSION_PATCHLEVEL}")
    set (GMP_VERSION "${GMP_VERSION_MAJOR}.${GMP_VERSION_MINOR}.${GMP_VERSION_PATCH}")
endif()

# Set GMP_LIBRARIES
#
find_library (GMP_LIBRARY NAMES gmp HINTS ${GMP_ROOT_HINTS})
set (GMP_LIBRARIES ${GMP_LIBRARY})

# Set GMP_FOUND
#
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (
    GMP
    REQUIRED_VARS GMP_INCLUDE_DIRS GMP_LIBRARIES
    VERSION_VAR GMP_VERSION
    )


#
# Find libgmpxx
#
if (GMP_FOUND)
    # Allow user to set GMPXX_ROOT_DIR to a custom GMPXX installation directory
    #
    set (GMPXX_ROOT_HINTS ${GMPXX_ROOT_DIR} ENV GMPXX_ROOT_DIR)

    # Set GMPXX_INCLUDE_DIRS
    #
    find_path (GMPXX_INCLUDE_DIR NAMES gmpxx.h ${GMPXX_ROOT_HINTS})
    set (GMPXX_INCLUDE_DIRS ${GMPXX_INCLUDE_DIR})

    # Set GMP_VERSION
    #
    if ("${GMPXX_INCLUDE_DIR}" STREQUAL "GMPXX_INCLUDE_DIR-NOTFOUND")
        message (STATUS "Could not find include file gmpxx.h")
    else()
        set (GMPXX_VERSION_MAJOR "${GMP_VERSION_MAJOR}")
        set (GMPXX_VERSION_MINOR "${GMP_VERSION_MINOR}")
        set (GMPXX_VERSION_PATCH "${GMP_VERSION_PATCH}")
        set (GMPXX_VERSION "${GMPXX_VERSION_MAJOR}.${GMPXX_VERSION_MINOR}.${GMPXX_VERSION_PATCH}")
    endif()

    # Set GMPXX_LIBRARIES
    #
    find_library (GMPXX_LIBRARY NAMES gmpxx HINTS ${GMPXX_ROOT_HINTS})
    set (GMPXX_LIBRARIES ${GMPXX_LIBRARY})

    # Set GMP_FOUND
    #
    include (FindPackageHandleStandardArgs)
    find_package_handle_standard_args (
        GMPXX
        REQUIRED_VARS GMPXX_INCLUDE_DIRS GMPXX_LIBRARIES
        VERSION_VAR GMPXX_VERSION
        )
else()
    set (GMPXX_FOUND False)
endif()

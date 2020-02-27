# Try to find LibFabric (https://github.com/ofiwg/libfabric)
# Once done this will define
#  LIBFABRIC_FOUND       - System has LibFabric
#  LIBFABRIC_INCLUDE_DIR - The LibFabric include directories
#  LIBFABRIC_LIBRARY     - The libraries needed to use LibFabric

# searhc prefix path
set(LIBFABRIC_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE STRING "Help cmake to find LibFabric library (https://github.com/ofiwg/libfabric).")

# check include
find_path(LIBFABRIC_INCLUDE_DIR rdma/fabric.h
	HINTS ${LIBFABRIC_PREFIX}/include)

# check lib
find_library(LIBFABRIC_LIBRARY NAMES fabric
	HINTS ${LIBFABRIC_PREFIX}/lib)

# setup found
if (LIBFABRIC_LIBRARY AND LIBFABRIC_INCLUDE_DIR)
	set(LIBFABRIC_FOUND ON)
endif()

# handle QUIET/REQUIRED
include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBFABRIC_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibFabric DEFAULT_MSG LIBFABRIC_LIBRARY LIBFABRIC_INCLUDE_DIR)

# Hide internal variables
mark_as_advanced(LIBFABRIC_INCLUDE_DIR LIBFABRIC_LIBRARY LIBFABRIC_FOUND)

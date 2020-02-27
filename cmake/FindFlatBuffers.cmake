# Try to find Flat Buffers (https://github.com/google/flatbuffers)
# Once done this will define
#  FLATBUFFERS_FOUND       - System has Flat Buffers
#  FLATBUFFERS_INCLUDE_DIR - The Flat Buffers include directories
#  FLATBUFFERS_LIBRARY     - The libraries needed to use Flat Buffers

# searhc prefix path
set(FLATBUFFERS_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE STRING "Help cmake to find Flat Buffers library (https://github.com/google/flatbuffers).")

# check include
find_path(FLATBUFFERS_INCLUDE_DIR rdma/fabric.h
	HINTS ${FLATBUFFERS_PREFIX}/include)

# check lib
find_library(FLATBUFFERS_LIBRARY NAMES fabric
	HINTS ${FLATBUFFERS_PREFIX}/lib)

# setup found
if (FLATBUFFERS_LIBRARY AND FLATBUFFERS_INCLUDE_DIR)
	set(FLATBUFFERS_FOUND ON)
endif()

# handle QUIET/REQUIRED
include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set FLATBUFFERS_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(FlatBuffers DEFAULT_MSG FLATBUFFERS_LIBRARY FLATBUFFERS_INCLUDE_DIR)

# Hide internal variables
mark_as_advanced(FLATBUFFERS_INCLUDE_DIR FLATBUFFERS_LIBRARY FLATBUFFERS_FOUND)

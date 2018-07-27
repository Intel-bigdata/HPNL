cmake_minimum_required (VERSION 2.8)
project (RDMA_MSG)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -g -pthread")

set(LOG ${PROJECT_SOURCE_DIR}/src/log)
set(UTIL ${PROJECT_SOURCE_DIR}/src/util)
set(DEMULTIPLEXER ${PROJECT_SOURCE_DIR}/src/demultiplexer)
set(CORE ${PROJECT_SOURCE_DIR}/src/core)
set(SERVICE ${PROJECT_SOURCE_DIR}/src/service)

set(DEMULTIPLEXER_FILE ${DEMULTIPLEXER}/Reactor.cc ${DEMULTIPLEXER}/EQEventDemultiplexer.cc ${DEMULTIPLEXER}/EQHandler.cc ${DEMULTIPLEXER}/CQEventDemultiplexer.cc)
set(LOG_FILE ${LOG}/NanoLog.cc)
set(CORE_FILE ${CORE}/FIConnection.cc ${CORE}/FIStack.cc ${CORE}/ConMgr.cc)
set(SERVICE_FILE ${SERVICE}/Service.cc ${SERVICE}/Server.cc ${SERVICE}/Client.cc)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(hpnl SHARED ${DEMULTIPLEXER_FILE} ${LOG_FILE} ${CORE_FILE} ${SERVICE_FILE})
set_target_properties(hpnl PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set(CMAKE_INSTALL_PREFIX "/usr/local")
install(TARGETS hpnl LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

file(GLOB INCLUDE_FILE ${PROJECT_SOURCE_DIR}/include/HPNL/*.h)
install(FILES ${INCLUDE_FILE} DESTINATION include/HPNL)

add_subdirectory(test)

cmake_minimum_required (VERSION 2.8)
project (RDMA_MSG)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -g -pthread")

set(TEST ${PROJECT_SOURCE_DIR}/test/ping-pong)
set(LOG ${PROJECT_SOURCE_DIR}/src/log)
set(UTIL ${PROJECT_SOURCE_DIR}/src/util)
set(DEMULTIPLEXER ${PROJECT_SOURCE_DIR}/src/demultiplexer)
set(CORE ${PROJECT_SOURCE_DIR}/src/core)

set(DEMULTIPLEXER_FILE ${DEMULTIPLEXER}/Reactor.cc ${DEMULTIPLEXER}/FIEventDemultiplexer.cc ${DEMULTIPLEXER}/CQHandler.cc ${DEMULTIPLEXER}/CMHandler.cc)
set(LOG_FILE ${LOG}/NanoLog.cc)
set(CORE_FILE ${CORE}/FIConnection.cc ${CORE}/FIStack.cc)

include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(server ${TEST}/server.cc ${DEMULTIPLEXER_FILE} ${LOG_FILE} ${CORE_FILE})
target_link_libraries(server fabric)

add_executable(client ${TEST}/client.cc ${DEMULTIPLEXER_FILE} ${LOG_FILE} ${CORE_FILE})
target_link_libraries(client fabric)

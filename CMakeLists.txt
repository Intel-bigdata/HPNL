cmake_minimum_required (VERSION 2.8)
project (HPNL)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -g -pthread -fPIC")

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(/usr/local/include)

set(UTIL ${PROJECT_SOURCE_DIR}/src/util)
set(DEMULTIPLEXER ${PROJECT_SOURCE_DIR}/src/demultiplexer)
set(CORE ${PROJECT_SOURCE_DIR}/src/core)
set(SERVICE ${PROJECT_SOURCE_DIR}/src/service)
set(EXTERNAL_SERVICE ${PROJECT_SOURCE_DIR}/src/external_service)
set(EXTERNAL_DEMULTIPLEXER ${PROJECT_SOURCE_DIR}/src/external_demultiplexer)

set(DEMULTIPLEXER_FILE ${DEMULTIPLEXER}/Proactor.cc ${DEMULTIPLEXER}/EqDemultiplexer.cc ${DEMULTIPLEXER}/EqHandler.cc ${DEMULTIPLEXER}/CqDemultiplexer.cc)
set(EXTERNAL_DEMULTIPLEXER_FILE ${EXTERNAL_DEMULTIPLEXER}/ExternalEqDemultiplexer.cc ${EXTERNAL_DEMULTIPLEXER}/ExternalCqDemultiplexer.cc)
set(CORE_FILE ${CORE}/FiConnection.cc ${CORE}/FiStack.cc)
set(SERVICE_FILE ${SERVICE}/Service.cc ${SERVICE}/Server.cc ${SERVICE}/Client.cc)
set(EXTERNAL_SERVICE_FILE ${EXTERNAL_SERVICE}/ExternalEqService.cc)

add_subdirectory(test)
add_subdirectory(java)

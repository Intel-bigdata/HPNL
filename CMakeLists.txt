# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

cmake_minimum_required (VERSION 2.8)
project (HPNL)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_COMPILER /usr/bin/g++)

# include custom FindXXX.cmake path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/)

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()
if(APPLE)
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -g -pthread -fPIC")
endif()
if(LINUX)
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -g -pthread -fPIC -fstack-protector-strong -fno-strict-overflow -fno-delete-null-pointer-checks -fwrapv")
endif()

find_package(Boost REQUIRED COMPONENTS system)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# search libfabric
find_package(LibFabric REQUIRED)
if (LIBFABRIC_FOUND)
  include_directories(${LIBFABRIC_INCLUDE_DIR})
endif()

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/Catch2/single_include)
include_directories(/usr/local/include)

if (JNI_FOUND)
  message (STATUS "JNI_INCLUDE_DIRS=${JNI_INCLUDE_DIRS}")
  message (STATUS "JNI_LIBRARIES=${JNI_LIBRARIES}")
endif()

if (APPLE)
  set(CMAKE_MACOSX_RPATH ON)
endif (APPLE)

set(UTIL ${PROJECT_SOURCE_DIR}/src/util)
set(DEMULTIPLEXER ${PROJECT_SOURCE_DIR}/src/demultiplexer)
set(CORE ${PROJECT_SOURCE_DIR}/src/core)
set(SERVICE ${PROJECT_SOURCE_DIR}/src/service)
set(EXTERNAL_SERVICE ${PROJECT_SOURCE_DIR}/src/external_service)
set(EXTERNAL_DEMULTIPLEXER ${PROJECT_SOURCE_DIR}/src/external_demultiplexer)
set(CHUNK ${PROJECT_SOURCE_DIR}/src/chunk)

set(DEMULTIPLEXER_FILE ${DEMULTIPLEXER}/Proactor.cc ${DEMULTIPLEXER}/EqDemultiplexer.cc ${DEMULTIPLEXER}/EqHandler.cc ${DEMULTIPLEXER}/CqDemultiplexer.cc ${DEMULTIPLEXER}/RdmCqDemultiplexer.cc)
set(EXTERNAL_DEMULTIPLEXER_FILE ${EXTERNAL_DEMULTIPLEXER}/ExternalEqDemultiplexer.cc ${EXTERNAL_DEMULTIPLEXER}/ExternalCqDemultiplexer.cc ${EXTERNAL_DEMULTIPLEXER}/ExternalRdmCqDemultiplexer.cc)
set(CORE_FILE ${CORE}/MsgConnection.cc ${CORE}/MsgStack.cc ${CORE}/RdmConnection.cc ${CORE}/RdmStack.cc)
set(SERVICE_FILE ${SERVICE}/Service.cc ${SERVICE}/Server.cc ${SERVICE}/Client.cc)
set(EXTERNAL_SERVICE_FILE ${EXTERNAL_SERVICE}/ExternalEqService.cc)
set(CHUNK_FILE ${CHUNK}/ChunkMgr.cc ${CHUNK}/ExternalChunkMgr.cc)

option(WITH_PSM2 "enable PSM2 provider" OFF)
if(WITH_PSM2)
  add_definitions(-DPSM2)
endif(WITH_PSM2)

option(WITH_VERBS "enable VERBS provider" OFF)
if(WITH_VERBS)
  add_definitions(-DVERBS)
endif(WITH_VERBS)

set(SUPPORT_JAVA FALSE)
option(WITH_JAVA "enable java interface" OFF)
if(WITH_JAVA)
  add_subdirectory(java)
  set(SUPPORT_JAVA TRUE)
endif()

add_subdirectory(src)
add_subdirectory(examples)

enable_testing()
add_test(NAME CHUNK_TEST COMMAND chunk_test)
add_test(NAME CORE_TEST COMMAND core_test)
add_test(NAME SERVICE_TEST COMMAND service_test)

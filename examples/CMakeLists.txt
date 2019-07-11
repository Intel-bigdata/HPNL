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

set(PING_PONG_TEST ${PROJECT_SOURCE_DIR}/examples/ping-pong)

add_executable(pingpong_server ${PING_PONG_TEST}/server.cc)
target_link_libraries(pingpong_server hpnl)
set_target_properties(pingpong_server
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/pingpong")

add_executable(pingpong_client ${PING_PONG_TEST}/client.cc)
target_link_libraries(pingpong_client hpnl)
set_target_properties(pingpong_client
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/pingpong")

set(CONNECTION_TEST ${PROJECT_SOURCE_DIR}/examples/connection)

add_executable(connection_server ${CONNECTION_TEST}/server.cc)
target_link_libraries(connection_server hpnl)
set_target_properties(connection_server
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/connection")

add_executable(connection_client ${CONNECTION_TEST}/client.cc)
target_link_libraries(connection_client hpnl)
set_target_properties(connection_client
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/connection")

set(RDM_TEST ${PROJECT_SOURCE_DIR}/examples/rdm)

add_executable(rdm_server ${RDM_TEST}/server.cc)
target_link_libraries(rdm_server hpnl)
set_target_properties(rdm_server
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/rdm")

add_executable(rdm_client ${RDM_TEST}/client.cc)
target_link_libraries(rdm_client hpnl)
set_target_properties(rdm_client
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/rdm")

option(WITH_PMEM "enable PMoF code" OFF)
if(WITH_PMEM)
  set(PMEM_TEST ${PROJECT_SOURCE_DIR}/examples/pmem)

  add_executable(pmem_server ${PMEM_TEST}/server.cc)
  target_link_libraries(pmem_server hpnl)
  set_target_properties(pmem_server
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/pmem")

  add_executable(pmem_client ${PMEM_TEST}/client.cc)
  target_link_libraries(pmem_client pmemobj hpnl)
  set_target_properties(pmem_client
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/pmem")
endif(WITH_PMEM)

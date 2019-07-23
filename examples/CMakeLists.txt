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

option(WITH_RMA_TEST "enable remote memory access test" OFF)
if(WITH_RMA_TEST)
  set(RMA_TEST ${PROJECT_SOURCE_DIR}/examples/rma)

  add_executable(rma_server ${RMA_TEST}/server.cc)
  target_link_libraries(rma_server hpnl flatbuffers)
  set_target_properties(rma_server
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/rma")

  add_executable(rma_client ${RMA_TEST}/client.cc)
  target_link_libraries(rma_client hpnl flatbuffers)
  set_target_properties(rma_client
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/rma")
endif(WITH_RMA_TEST)


option(WITH_PMEM_TEST "enable PMoF code" OFF)
if(WITH_PMEM_TEST)
  set(PMEM_TEST ${PROJECT_SOURCE_DIR}/examples/pmof)

  add_executable(pmof_server ${PMEM_TEST}/server.cc)
  target_link_libraries(pmof_server pmemobj hpnl)
  set_target_properties(pmof_server
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/pmof")

  add_executable(pmof_client ${PMEM_TEST}/client.cc)
  target_link_libraries(pmof_client pmemobj hpnl)
  set_target_properties(pmof_client
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/pmof")
endif(WITH_PMEM_TEST)

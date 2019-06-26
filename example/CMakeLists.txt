set(PING_PONG_TEST ${PROJECT_SOURCE_DIR}/example/ping-pong)

add_executable(pingpong_server ${PING_PONG_TEST}/server.cc)
target_link_libraries(pingpong_server fabric hpnl)
set_target_properties(pingpong_server
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/pingpong")

add_executable(pingpong_client ${PING_PONG_TEST}/client.cc)
target_link_libraries(pingpong_client fabric hpnl)
set_target_properties(pingpong_client
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/pingpong")

set(CONNECTION_TEST ${PROJECT_SOURCE_DIR}/example/connection)

add_executable(connection_server ${CONNECTION_TEST}/server.cc)
target_link_libraries(connection_server fabric hpnl)
set_target_properties(connection_server
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/connection")

add_executable(connection_client ${CONNECTION_TEST}/client.cc)
target_link_libraries(connection_client fabric hpnl)
set_target_properties(connection_client
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/connection")

set(RDM_TEST ${PROJECT_SOURCE_DIR}/example/rdm)

add_executable(rdm_server ${RDM_TEST}/server.cc)
target_link_libraries(rdm_server fabric hpnl)
set_target_properties(rdm_server
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/rdm")

add_executable(rdm_client ${RDM_TEST}/client.cc)
target_link_libraries(rdm_client fabric hpnl)
set_target_properties(rdm_client
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/rdm")

option(WITH_PMEM "enable PMoF code" OFF)
if(WITH_PMEM)
  set(PMEM_TEST ${PROJECT_SOURCE_DIR}/example/pmem)

  add_executable(pmem_server ${PMEM_TEST}/server.cc)
  target_link_libraries(pmem_server fabric pmemobj hpnl)
  set_target_properties(pmem_server
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/pmem")

  add_executable(pmem_client ${PMEM_TEST}/client.cc)
  target_link_libraries(pmem_client fabric pmemobj hpnl)
  set_target_properties(pmem_client
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/pmem")
endif(WITH_PMEM)

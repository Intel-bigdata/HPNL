set(PING_PONG_TEST ${PROJECT_SOURCE_DIR}/test/ping-pong)

add_executable(pingpong_server ${PING_PONG_TEST}/server.cc ${DEMULTIPLEXER_FILE} ${LOG_FILE} ${CORE_FILE} ${SERVICE_FILE} ${EXTERNAL_DEMULTIPLEXER_FILE} ${EXTERNAL_SERVICE_FILE})
target_link_libraries(pingpong_server fabric)
set_target_properties(pingpong_server
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/pingpong")

add_executable(pingpong_client ${PING_PONG_TEST}/client.cc ${DEMULTIPLEXER_FILE} ${LOG_FILE} ${CORE_FILE} ${SERVICE_FILE} ${EXTERNAL_DEMULTIPLEXER_FILE} ${EXTERNAL_SERVICE_FILE})
target_link_libraries(pingpong_client fabric)
set_target_properties(pingpong_client
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/pingpong")

set(CONNECTION_TEST ${PROJECT_SOURCE_DIR}/test/connection)

add_executable(connection_server ${CONNECTION_TEST}/server.cc ${DEMULTIPLEXER_FILE} ${LOG_FILE} ${CORE_FILE} ${SERVICE_FILE} ${EXTERNAL_DEMULTIPLEXER_FILE} ${EXTERNAL_SERVICE_FILE})
target_link_libraries(connection_server fabric)
set_target_properties(connection_server
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/connection")

add_executable(connection_client ${CONNECTION_TEST}/client.cc ${DEMULTIPLEXER_FILE} ${LOG_FILE} ${CORE_FILE} ${SERVICE_FILE} ${EXTERNAL_DEMULTIPLEXER_FILE} ${EXTERNAL_SERVICE_FILE})
target_link_libraries(connection_client fabric)
set_target_properties(connection_client
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/connection")


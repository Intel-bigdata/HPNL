add_executable(core_test ${PROJECT_SOURCE_DIR}/src/test/CoreTest.cc ${CORE_FILE} ${CHUNK_FILE})
target_link_libraries(core_test fabric boost_system)
set_target_properties(core_test
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/test")

add_executable(service_test ${PROJECT_SOURCE_DIR}/src/test/ServiceTest.cc
        ${CORE_FILE} ${SERVICE_FILE} ${DEMULTIPLEXER_FILE} ${CHUNK_FILE})
target_link_libraries(service_test fabric boost_system)
set_target_properties(service_test
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/test")

add_executable(chunk_test ${PROJECT_SOURCE_DIR}/src/test/ChunkPoolTest.cc ${CHUNK_FILE} chunk/ExternalChunkMgr.cc)
target_link_libraries(chunk_test fabric boost_system)
set_target_properties(chunk_test
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/test")

if(NOT ${SUPPORT_JAVA})
  add_library(hpnl SHARED ${DEMULTIPLEXER_FILE} ${CORE_FILE} ${SERVICE_FILE} ${EXTERNAL_DEMULTIPLEXER_FILE} ${EXTERNAL_SERVICE_FILE} ${CHUNK_FILE})
  target_link_libraries(hpnl fabric boost_system)
  set_target_properties(hpnl PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

  set(CMAKE_INSTALL_PREFIX "/usr/local")
  install(TARGETS hpnl LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

  file(GLOB INCLUDE_FILE ${PROJECT_SOURCE_DIR}/include/HPNL/*.h)
  install(FILES ${INCLUDE_FILE} DESTINATION include/HPNL)
endif()

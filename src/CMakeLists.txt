add_executable(core_test ${PROJECT_SOURCE_DIR}/src/test/coreTest.cc ${CORE_FILE})
target_link_libraries(core_test fabric)
set_target_properties(core_test
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/test")

if(NOT ${SUPPORT_JAVA})
  add_library(hpnl SHARED ${DEMULTIPLEXER_FILE} ${CORE_FILE} ${SERVICE_FILE} ${EXTERNAL_DEMULTIPLEXER_FILE} ${EXTERNAL_SERVICE_FILE})
  target_link_libraries(hpnl fabric)
  set_target_properties(hpnl PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

  set(CMAKE_INSTALL_PREFIX "/usr/local")
  install(TARGETS hpnl LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

  file(GLOB INCLUDE_FILE ${PROJECT_SOURCE_DIR}/include/HPNL/*.h)
  install(FILES ${INCLUDE_FILE} DESTINATION include/HPNL)
endif()

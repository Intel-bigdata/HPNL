add_executable(core_test ${PROJECT_SOURCE_DIR}/src/test/coreTest.cc ${CORE_FILE})
target_link_libraries(core_test fabric)
set_target_properties(core_test
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")


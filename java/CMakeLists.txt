set(NATIVE ${PROJECT_SOURCE_DIR}/java/native)
set(NATIVE_FILE ${NATIVE}/com_intel_hpnl_core_EqService.cc ${NATIVE}/com_intel_hpnl_core_Connection.cc ${NATIVE}/com_intel_hpnl_core_CqService.cc ${NATIVE}/com_intel_hpnl_core_Utils.cc ${NATIVE}/com_intel_hpnl_core_RdmService.cc ${NATIVE}/com_intel_hpnl_core_RdmConnection.cc)

set(JNI_INCLUDE "$ENV{JAVA_HOME}/include" "$ENV{JAVA_HOME}/include/linux")
include_directories(${JNI_INCLUDE})

add_library(hpnl SHARED ${DEMULTIPLEXER_FILE} ${CORE_FILE} ${SERVICE_FILE} ${EXTERNAL_DEMULTIPLEXER_FILE} ${EXTERNAL_SERVICE_FILE} ${NATIVE_FILE})
target_link_libraries(hpnl fabric)
set_target_properties(hpnl PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set(CMAKE_INSTALL_PREFIX "/usr/local")
install(TARGETS hpnl LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

file(GLOB INCLUDE_FILE ${PROJECT_SOURCE_DIR}/include/HPNL/*.h)
install(FILES ${INCLUDE_FILE} DESTINATION include/HPNL)


# Install script for directory: /code-repo/HPNL/HPNL/java

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libhpnl.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libhpnl.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libhpnl.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/code-repo/HPNL/HPNL/build/lib/libhpnl.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libhpnl.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libhpnl.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libhpnl.so")
    endif()
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/HPNL" TYPE FILE FILES
    "/code-repo/HPNL/HPNL/include/HPNL/Client.h"
    "/code-repo/HPNL/HPNL/include/HPNL/Connection.h"
    "/code-repo/HPNL/HPNL/include/HPNL/CQExternalDemultiplexer.h"
    "/code-repo/HPNL/HPNL/include/HPNL/Log.h"
    "/code-repo/HPNL/HPNL/include/HPNL/Service.h"
    "/code-repo/HPNL/HPNL/include/HPNL/Reactor.h"
    "/code-repo/HPNL/HPNL/include/HPNL/FIStack.h"
    "/code-repo/HPNL/HPNL/include/HPNL/Callback.h"
    "/code-repo/HPNL/HPNL/include/HPNL/ExternalCqService.h"
    "/code-repo/HPNL/HPNL/include/HPNL/Handle.h"
    "/code-repo/HPNL/HPNL/include/HPNL/ConMgr.h"
    "/code-repo/HPNL/HPNL/include/HPNL/Server.h"
    "/code-repo/HPNL/HPNL/include/HPNL/CQEventDemultiplexer.h"
    "/code-repo/HPNL/HPNL/include/HPNL/NanoLog.h"
    "/code-repo/HPNL/HPNL/include/HPNL/EventDemultiplexer.h"
    "/code-repo/HPNL/HPNL/include/HPNL/EQExternalDemultiplexer.h"
    "/code-repo/HPNL/HPNL/include/HPNL/EQHandler.h"
    "/code-repo/HPNL/HPNL/include/HPNL/ThreadWrapper.h"
    "/code-repo/HPNL/HPNL/include/HPNL/ExternalEqServiceBufMgr.h"
    "/code-repo/HPNL/HPNL/include/HPNL/Ptr.h"
    "/code-repo/HPNL/HPNL/include/HPNL/EventHandler.h"
    "/code-repo/HPNL/HPNL/include/HPNL/ExternalEqService.h"
    "/code-repo/HPNL/HPNL/include/HPNL/FIConnection.h"
    "/code-repo/HPNL/HPNL/include/HPNL/EventType.h"
    "/code-repo/HPNL/HPNL/include/HPNL/Common.h"
    "/code-repo/HPNL/HPNL/include/HPNL/EQEventDemultiplexer.h"
    "/code-repo/HPNL/HPNL/include/HPNL/BufMgr.h"
    )
endif()


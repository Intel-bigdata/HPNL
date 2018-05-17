cmake_minimum_required (VERSION 2.8)
project (RDMA_MESSENGER)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -g -pthread")
#set(Boost_INCLUDE_DIR /usr/local/include)
#set(Boost_LIBRARY_DIR /usr/local/lib)

#include_directories(${Boost_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR})
#link_directories(${Boost_LIBRARY_DIR})

#find_package(Boost 1.40 COMPONENTS system REQUIRED)

add_executable(server server.cc Reactor.cc CQHandler.cc FIConnection.cc FIStack.cc CMHandler.cc FIEventDemultiplexer.cc FIStack.cc)
target_link_libraries(server fabric)

add_executable(client client.cc Reactor.cc CQHandler.cc FIConnection.cc FIStack.cc CMHandler.cc FIEventDemultiplexer.cc FIStack.cc)
target_link_libraries(client fabric)

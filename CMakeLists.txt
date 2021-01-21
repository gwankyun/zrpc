cmake_minimum_required(VERSION 3.19)

set(zrpc zrpc)
project(${zrpc})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
include(CMakePrintHelpers)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/W4;/MP;/Zc:__cplusplus;>")
add_compile_definitions("ZRPC_HAS_CXX_11=0")

if(NOT dbg_macro_ROOT)
  message(FATAL_ERROR "must set dbg_macro_ROOT")
endif()

if(NOT asio_ROOT)
  message(FATAL_ERROR "must set asio_ROOT")
endif()

if(NOT apply_ROOT)
  message(FATAL_ERROR "must set apply_ROOT")
endif()

set(zrpc_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
find_package(zrpc REQUIRED)
cmake_print_variables(zrpc_INCLUDE_DIRS)

find_package(apply REQUIRED)

find_package(Boost REQUIRED)

find_package(msgpack REQUIRED)

find_package(dbg_macro REQUIRED)

add_executable(${zrpc})
target_sources(${zrpc} PRIVATE
  src/main.cpp
  include/zrpc.hpp
  include/zrpc/zrpc.h
  include/zrpc/detail.hpp
  include/zrpc/client.hpp
  include/zrpc/server.hpp
  include/zrpc/serialization.hpp
  cmake/Findzrpc.cmake
)
target_link_libraries(${zrpc} zrpc::zrpc)
target_compile_features(${zrpc} PRIVATE cxx_std_17)

set(server server)
add_executable(${server})
target_sources(${server} PRIVATE src/server.cpp include/log.hpp)
target_compile_features(${server} PRIVATE cxx_std_17)
target_link_libraries(${server} zrpc::zrpc dbg_macro::dbg_macro)

set(client client)
add_executable(${client})
target_sources(${client} PRIVATE src/client.cpp include/log.hpp)
target_compile_features(${client} PRIVATE cxx_std_17)
target_link_libraries(${client} zrpc::zrpc dbg_macro::dbg_macro)

set(async_client async_client)
add_executable(${async_client})
target_sources(${async_client} PRIVATE src/async_client.cpp)
target_compile_features(${async_client} PRIVATE cxx_std_17)
target_link_libraries(${async_client} zrpc::zrpc dbg_macro::dbg_macro)

cmake_minimum_required(VERSION 3.21)

set(zrpc zrpc)
project(${zrpc})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(ZRPC_USE_BOOST_ASIO "" ON)
option(ZRPC_SERVER_USE_CXX_11 "" ON)
option(ZRPC_CLIENT_USE_CXX_11 "" ON)
option(ZRPC_HAS_CXX_11 "" ON)

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
include(CMakePrintHelpers)
include(GNUInstallDirs)
include(common)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/W4;/MP;/Zc:__cplusplus;>")
add_compile_definitions("ZRPC_USE_BOOST_ASIO=$<BOOL:${ZRPC_USE_BOOST_ASIO}>")

# get_filename_component(parent ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
# cmake_print_variables(parent)

# if(NOT dbg_macro_ROOT)
#   message(FATAL_ERROR "must set dbg_macro_ROOT")
# endif()

# if(NOT asio_ROOT)
#   message(FATAL_ERROR "must set asio_ROOT")
# endif()

# if(NOT apply_ROOT)
#   message(FATAL_ERROR "must set apply_ROOT")
# endif()

# set(zrpc_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
# find_package(zrpc REQUIRED)
# cmake_print_variables(zrpc_INCLUDE_DIRS)

# find_package(apply CONFIG REQUIRED)

# find_package(Boost CONFIG REQUIRED)

# find_package(msgpack CONFIG REQUIRED)

# find_package(dbg_macro CONFIG REQUIRED)

##
set(target zrpc)

add_library(${target} INTERFACE)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/zrpc" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/zrpc.hpp" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

target_include_directories(${target}
  INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

find_package(Boost REQUIRED COMPONENTS context)

# set(has_include_DIR config/has_include/lib/cmake/has_include)
# find_package(has_include CONFIG REQUIRED)
# add_subdirectory(has_include)

if(NOT(TARGET has_include) AND (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/has_include))
  add_subdirectory(has_include)
endif()

# add_subdirectory(third_party/has_include)

# find_package(preprocessor CONFIG REQUIRED)
# add_subdirectory(preprocessor)

if(NOT(TARGET preprocessor) AND (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/preprocessor))
  add_subdirectory(preprocessor)
endif()

# find_package(apply CONFIG REQUIRED)
add_subdirectory(apply)
# find_package(msgpack CONFIG REQUIRED)
# find_package(msgpack-cxx CONFIG REQUIRED)
find_package(msgpackc-cxx CONFIG REQUIRED)

target_link_directories(${target} INTERFACE ${Boost_LIBRARY_DIRS})

# target_link_libraries(${target} INTERFACE apply::apply Boost::headers has_include::has_include preprocessor::preprocessor msgpackc-cxx Boost::context)
target_link_libraries(${target} INTERFACE
  apply
  Boost::headers
  # has_include
  $<TARGET_NAME_IF_EXISTS:has_include>
  # preprocessor
  $<TARGET_NAME_IF_EXISTS:preprocessor>
  msgpackc-cxx
  Boost::context
)

target_compile_definitions(${target} INTERFACE "ENABLE_HAS_INCLUDE_TEST=0;ZRPC_HAS_CXX_11=$<BOOL:${ZRPC_HAS_CXX_11}>")

target_install(${target} "0.0.1" ${target})
##

add_executable(main)
target_sources(main PRIVATE
  src/main.cpp
  include/zrpc.hpp
  include/zrpc/zrpc.h
  include/zrpc/detail.hpp
  include/zrpc/client.hpp
  include/zrpc/server.hpp
  include/zrpc/serialization.hpp
  include/zrpc/asio.hpp
  include/zrpc/coroutine.hpp
  cmake/Findzrpc.cmake
  cmake/common.cmake
)
target_link_libraries(main zrpc)
target_compile_features(main PRIVATE cxx_std_20)

cmake_print_variables(Boost_LIBRARY_DIRS)

set(server server)
add_executable(${server})
target_sources(${server} PRIVATE src/server.cpp include/log.hpp)
target_compile_features(${server} PRIVATE cxx_std_20)
target_link_directories(${server} PRIVATE ${Boost_LIBRARY_DIRS})
target_link_libraries(${server} zrpc)
target_compile_definitions(${server} PRIVATE "ZRPC_HAS_CXX_11=$<BOOL:${ZRPC_SERVER_USE_CXX_11}>")

set(client client)
add_executable(${client})
target_sources(${client} PRIVATE src/client.cpp include/log.hpp)
target_compile_features(${client} PRIVATE cxx_std_20)
target_link_libraries(${client} zrpc)
target_compile_definitions(${client} PRIVATE "ZRPC_HAS_CXX_11=$<BOOL:${ZRPC_CLIENT_USE_CXX_11}>")

set(async_client async_client)
add_executable(${async_client})
target_sources(${async_client} PRIVATE src/async_client.cpp)
target_compile_features(${async_client} PRIVATE cxx_std_20)
target_link_libraries(${async_client} zrpc)

find_package(benchmark)

if(benchmark_FOUND)
  set(benchmark benchmark)
  add_executable(${benchmark})
  target_sources(${benchmark} PRIVATE src/benchmark.cpp)
  target_compile_features(${benchmark} PRIVATE cxx_std_20)
  target_link_libraries(${benchmark} zrpc benchmark::benchmark)
endif()

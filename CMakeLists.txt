cmake_minimum_required(VERSION 3.21)

set(zrpc zrpc)
project(${zrpc})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
include(CMakePrintHelpers)
include(GNUInstallDirs)
include(common)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/W4;/MP;/Zc:__cplusplus;>")
add_compile_definitions("ZRPC_HAS_CXX_11=1")

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

set(Boost_ROOT "C:/Program Files/boost_1_77_0")

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

find_package(Boost REQUIRED)

# set(has_include_DIR config/has_include/lib/cmake/has_include)
find_package(has_include CONFIG REQUIRED)

# add_subdirectory(third_party/has_include)

find_package(preprocessor CONFIG REQUIRED)
find_package(apply CONFIG REQUIRED)
find_package(msgpack CONFIG REQUIRED)

target_link_libraries(${target} INTERFACE apply::apply Boost::headers has_include::has_include preprocessor::preprocessor msgpackc-cxx)

target_compile_definitions(${target} INTERFACE ENABLE_HAS_INCLUDE_TEST=0)

target_install(${target} "0.0.1")
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
  cmake/Findzrpc.cmake
)
target_link_libraries(main zrpc)
target_compile_features(main PRIVATE cxx_std_17)

set(server server)
add_executable(${server})
target_sources(${server} PRIVATE src/server.cpp include/log.hpp)
target_compile_features(${server} PRIVATE cxx_std_17)
target_link_libraries(${server} zrpc)

set(client client)
add_executable(${client})
target_sources(${client} PRIVATE src/client.cpp include/log.hpp)
target_compile_features(${client} PRIVATE cxx_std_17)
target_link_libraries(${client} zrpc)

set(async_client async_client)
add_executable(${async_client})
target_sources(${async_client} PRIVATE src/async_client.cpp)
target_compile_features(${async_client} PRIVATE cxx_std_17)
target_link_libraries(${async_client} zrpc)

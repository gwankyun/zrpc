# cmake最低版本
cmake_minimum_required(VERSION 3.19)

# 啟用IDE目錄功能
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 包含模塊
set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
include(CMakePrintHelpers)

# 輸出目錄
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# 項目名
set(zrpc zrpc)
project(${zrpc})

# 設置編譯選項
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/W4;/MP;/Zc:__cplusplus;>")

set(zrpc_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
find_package(zrpc REQUIRED)
cmake_print_variables(zrpc_INCLUDE_DIRS)

# set(apply_ROOT "D:/local/apply")
find_package(apply REQUIRED)

# 引入Boost
find_package(Boost REQUIRED)

# 引入msgpack
find_package(msgpack REQUIRED)

add_executable(${zrpc})
target_sources(${zrpc} PRIVATE
  src/main.cpp
  include/zrpc.hpp
  include/zrpc/zrpc.h
  include/zrpc/detail.hpp
  cmake/Findzrpc.cmake
)
# target_include_directories(${zrpc} PRIVATE include D:/source/repos/cmake-example/include)
target_link_libraries(${zrpc} zrpc::zrpc)

# 執行文件
set(server server)
add_executable(${server})

# 添加源碼
target_sources(${server} PRIVATE src/server.cpp include/log.hpp)

# 頭文件目錄
# target_include_directories(${server} PRIVATE inclu)

# 使用C++17標準
target_compile_features(${server} PRIVATE cxx_std_17)

# 鏈接庫
target_link_libraries(${server} zrpc::zrpc)

# 執行文件
set(client client)
add_executable(${client})

# 添加源碼
target_sources(${client} PRIVATE src/client.cpp include/log.hpp)

# 頭文件目錄
# target_include_directories(${client} PRIVATE include D:/source/repos/cmake-example/include)

# 使用C++17標準
target_compile_features(${client} PRIVATE cxx_std_17)

# 鏈接庫
target_link_libraries(${client} zrpc::zrpc)


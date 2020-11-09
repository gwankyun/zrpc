# cmake最低版本
cmake_minimum_required(VERSION 3.18)

# 啟用IDE目錄功能
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 包含模塊
include(CMakePrintHelpers)

# 輸出目錄
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# 項目名
project(zrpc)

# 設置編譯選項
add_compile_options(
  "$<$<CXX_COMPILER_ID:MSVC>:/W4;/MP>")

find_package(Boost 1.74.0 REQUIRED)

# 執行文件
add_executable(server)

# 添加源碼
target_sources(server PRIVATE src/server.cpp include/log.hpp include/zrpc.hpp)

# 頭文件目錄
target_include_directories(server
  PRIVATE
    include
    ${Boost_INCLUDE_DIRS})

target_compile_features(server PRIVATE cxx_std_17)

# 執行文件
add_executable(client)

# 添加源碼
target_sources(client PRIVATE src/client.cpp include/log.hpp include/zrpc.hpp)

# 頭文件目錄
target_include_directories(client
  PRIVATE
    D:/local/boost_1_74_0/tmp
    include
    ${Boost_INCLUDE_DIRS})

target_compile_features(client PRIVATE cxx_std_17)

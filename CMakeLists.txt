cmake_minimum_required(VERSION 4.0)

PROJECT(mh_p1 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Para el LSP de nvim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB C_SOURCES "src/*.cpp")
include_directories("common" "inc")

add_executable(main "main.cpp" ${C_SOURCES})


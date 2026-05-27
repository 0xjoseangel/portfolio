cmake_minimum_required(VERSION 4.0)

PROJECT(mh_p1 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimizaciones agresivas
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# OpenMP para paralelización
find_package(OpenMP REQUIRED)

# Para el LSP de nvim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB C_SOURCES "src/*.cpp")
include_directories("common" "inc")

add_executable(main "main.cpp" ${C_SOURCES})
target_link_libraries(main PUBLIC OpenMP::OpenMP_CXX)

add_executable(study_blx_alpha "main_blx_alpha.cpp" ${C_SOURCES})
target_link_libraries(study_blx_alpha PUBLIC OpenMP::OpenMP_CXX)

add_executable(study_sa_params "main_sa_params.cpp" ${C_SOURCES})
target_link_libraries(study_sa_params PUBLIC OpenMP::OpenMP_CXX)

add_executable(main_p3 "main_p3.cpp" ${C_SOURCES})
target_link_libraries(main_p3 PUBLIC OpenMP::OpenMP_CXX)

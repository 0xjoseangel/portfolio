cmake_minimum_required(VERSION 4.0)

PROJECT(mh_p1 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Para el LSP de nvim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB C_SOURCES "src/*.cpp")
include_directories("common" "inc")

add_executable(main "main.cpp" ${C_SOURCES})

# ============================================================
#  Google Test
# ============================================================
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

# Ruta absoluta a los datos para que los tests la encuentren siempre
get_filename_component(DATA_DIR "${CMAKE_SOURCE_DIR}/data" ABSOLUTE)

file(GLOB TEST_SOURCES "test/*.cpp")
add_executable(all_tests ${TEST_SOURCES} ${C_SOURCES})
target_include_directories(all_tests PRIVATE "common" "inc")
target_compile_definitions(all_tests PRIVATE DATA_DIR="${DATA_DIR}")
target_link_libraries(all_tests GTest::gtest_main)

gtest_discover_tests(all_tests)

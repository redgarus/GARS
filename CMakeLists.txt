cmake_minimum_required(VERSION 3.20.0)

project(GARScript VERSION 2.0 LANGUAGES CXX)

# add_compile_options(-fsanitize=undefined)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER clang++)

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build")
set(TESTS ${CMAKE_SOURCE_DIR}/tests)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FILENAMES)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

enable_testing()

# LLLLLVM CON
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
llvm_map_components_to_libnames(llvm_libs -18)

add_executable(llvm_test tests/test.cpp)

target_link_libraries(llvm_test PUBLIC ${llvm_libs})

add_executable(compiler src/compiler.cpp src/visitor.cpp src/lexer.cpp src/parser.cpp src/type.cpp src/codegen.cpp)

target_link_libraries(compiler PUBLIC ${llvm_libs})

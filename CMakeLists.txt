cmake_minimum_required(VERSION 3.20.0)

project(GARScript VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build")
set(TESTS ${CMAKE_SOURCE_DIR}/tests)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FILENAMES)

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

target_link_libraries(llvm_test PRIVATE ${llvm_libs})

add_library(lexer src/lexer/lexer.cpp)
add_library(ast src/ast/ast.cpp src/ast/code.cpp)
add_library(parser src/parser/parser.cpp)

set(FILENAMES ${FILENAMES} "libparser.a")
set(FILENAMES ${FILENAMES} "liblexer.a")
set(FILENAMES ${FILENAMES} "libast.a")

add_executable(compiler src/compiler.cpp)

target_link_libraries(ast PUBLIC ${llvm_libs})
target_link_libraries(parser PUBLIC lexer ast)
target_link_libraries(compiler PUBLIC parser)

file(GLOB testFiles "${TESTS}/samples/*.gars")

foreach(file ${testFiles})
    get_filename_component(fileNameEXT ${file} NAME)
    get_filename_component(fileName ${file} NAME_WE)

    # set(FILENAMES ${FILENAMES} "${CMAKE_SOURCE_DIR}/${fileName}")
    # set(FILENAMES ${FILENAMES} "${CMAKE_SOURCE_DIR}/${fileName}.o")
    message("./build/compiler ${file}")
    add_test(${fileNameEXT} bash -c "./build/compiler ${TESTS}/samples/${fileNameEXT}")

endforeach()

add_custom_target(clear COMMAND rm -f ${FILENAMES})

cmake_minimum_required(VERSION 3.7...3.22)

# Fallback for versions of CMake that don't support version range syntax.
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(Kaleidoscope)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# C++ string formatting library that is a superset of the formatting library standardized in C++20.
find_package(fmt)

# TODO: Uncomment once using LLVM libs.
# find_package(LLVM REQUIRED CONFIG)
# message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
# message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Find the libraries that correspond to the LLVM components
# that we wish to use
# llvm_map_components_to_libnames(llvm_libs support core irreader) # libc++)

set(SOURCES
  "${CMAKE_SOURCE_DIR}/src/error.cpp"
  "${CMAKE_SOURCE_DIR}/src/lexer.cpp"
  "${CMAKE_SOURCE_DIR}/src/main.cpp"
  "${CMAKE_SOURCE_DIR}/src/parser.cpp"
  "${CMAKE_SOURCE_DIR}/src/module.cpp"
)


# Inline testcases and testing setup in main are stripped in release builds.
add_executable(kal ${SOURCES})
target_compile_definitions(kal PRIVATE $<$<CONFIG:Release>:DOCTEST_CONFIG_DISABLE>)
target_compile_options(
  kal
  PRIVATE
  -fvisibility=hidden
  -Werror
  -Wall
  -Werror=strict-prototypes
  -Werror=old-style-definition
  -Werror=missing-prototypes
  -Wstrict-aliasing
)
target_include_directories(kal PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(kal PRIVATE fmt::fmt) # ${llvm_libs})

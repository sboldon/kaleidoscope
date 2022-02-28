cmake_minimum_required(VERSION 3.7...3.22)

# Fallback for versions of CMake that don't support version range syntax.
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(Kaleidoscope)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# C++ formatted output library standardized in C++20 but not yet implemented in all compilers.
find_package(fmt)

set(SOURCES
  "${CMAKE_SOURCE_DIR}/src/error.cpp"
  "${CMAKE_SOURCE_DIR}/src/lexer.cpp"
  "${CMAKE_SOURCE_DIR}/src/main.cpp"
  "${CMAKE_SOURCE_DIR}/src/parser.cpp"
  "${CMAKE_SOURCE_DIR}/src/srcfile.cpp"
)

add_executable(kscope ${SOURCES})
target_compile_options(kscope PUBLIC -Werror -Wall -Werror=strict-prototypes -Werror=old-style-definition -Werror=missing-prototypes)
target_include_directories(kscope PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(kscope PRIVATE fmt::fmt)

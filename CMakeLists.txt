cmake_minimum_required(VERSION 3.5)
project(SATSolver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

add_custom_command(OUTPUT parser.cpp parser.hpp
				   DEPENDS ${CMAKE_SOURCE_DIR}/parser.y
				   COMMAND ${BISON_EXECUTABLE} -d --output parser.cpp ${CMAKE_SOURCE_DIR}/parser.y
				   COMMENT "Generating parser.cpp")

add_custom_command(OUTPUT lexer.cpp
				   DEPENDS ${CMAKE_SOURCE_DIR}/lexer.l ${CMAKE_BINARY_DIR}/parser.hpp
				   COMMAND ${FLEX_EXECUTABLE} -o lexer.cpp ${CMAKE_SOURCE_DIR}/lexer.l
				   COMMENT "Generating the lexer")

add_executable(satsolver satsolver.cpp parser.hpp parser.cpp lexer.cpp)

target_include_directories(satsolver PRIVATE ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR})

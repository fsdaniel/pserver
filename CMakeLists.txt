cmake_minimum_required(VERSION 3.4)
project(PServer)

if(${CMAKE_COMPILER_IS_GNUCXX})
	add_definitions("-std=gnu++14")
endif()

find_package(Boost 1.60.0 REQUIRED COMPONENTS system)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include)
set(SRC_FILES src/globals.cpp src/main.cpp src/server.cpp src/connection.cpp)

add_executable(pserver ${SRC_FILES})
target_link_libraries(pserver ${Boost_LIBRARIES})

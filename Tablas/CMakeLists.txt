cmake_minimum_required(VERSION 3.16)
project(incidents_analytics CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


find_package(Python3 COMPONENTS Interpreter Development REQUIRED)


find_path(SQLITE3_INCLUDE_DIR sqlite3.h)
find_library(SQLITE3_LIBRARY sqlite3)
if (NOT SQLITE3_INCLUDE_DIR OR NOT SQLITE3_LIBRARY)
message(FATAL_ERROR "sqlite3 dev files not found")
endif()


include_directories(${SQLITE3_INCLUDE_DIR})
include_directories(${Python3_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/third_party)


add_executable(incidents_analytics
src/main.cpp
src/db_loader.cpp
src/analytics.cpp
src/chart.cpp
)


target_link_libraries(incidents_analytics
${SQLITE3_LIBRARY}
${Python3_LIBRARIES}
)
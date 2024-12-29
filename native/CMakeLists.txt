cmake_minimum_required(VERSION 3.5)
project(nullunits)
set(CMAKE_C_STANDARD 11)

include(FetchContent)
# set(FETCHCONTENT_QUIET OFF)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
  set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
endif()

if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
  set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
endif()

# enable colors for ninja
set(CMAKE_COLOR_DIAGNOSTICS ON)

# set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(wamr REQUIRED)
find_package(SOUNDIO REQUIRED)
find_package(liblo REQUIRED)

file(GLOB_RECURSE NULLUNIT_SOURCES src/*.c)
add_executable(${PROJECT_NAME} ${NULLUNIT_SOURCES})
target_link_libraries(${PROJECT_NAME} wamr ${SOUNDIO_LIBRARY} ${LIBLO_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${SOUNDIO_INCLUDE_DIR} ${LIBLO_INCLUDE_DIRS})

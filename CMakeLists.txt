cmake_minimum_required(VERSION 3.8)
project(Simple_P2P)

set(THREADS_PREFER_PTHREAD_FLAG OFF)

find_package(Threads REQUIRED)

find_package(Boost REQUIRED system program_options)

find_package(PkgConfig REQUIRED)

pkg_check_modules(TBB tbb REQUIRED)

set(EXTERNAL_LIBS "stdc++fs" "Boost::system" "Boost::program_options" "Threads::Threads" "tbb")

include_directories("${PROJECT_SOURCE_DIR}/include")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

cmake_policy(SET CMP0069 NEW)
include(CheckIPOSupported)
check_ipo_supported()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Og -g -Wshadow -Wall -pedantic -Wextra -Wold-style-cast")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fdevirtualize -finline-functions -funroll-loops")

set(EXECUTABLE_NAME ${PROJECT_NAME})

file(GLOB SOURCE_FILES "src/*.cpp")
file(GLOB INCLUDE_FILES "include/*.h")

add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES} ${INCLUDE_FILES})

target_link_libraries(${EXECUTABLE_NAME} ${EXTERNAL_LIBS})

project(RACOS)
cmake_minimum_required(VERSION 3.5.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# add include path
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

# add src path
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src src)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(racos ${src})
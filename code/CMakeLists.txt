cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CXX_STANDARD 17)

project(rtp_sim)

add_executable(
    ${PROJECT_NAME}
    src/main.cpp
)


find_package(HighFive REQUIRED)
find_package(RngStreams REQUIRED)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    HighFive
    PRIVATE 
    RngStreams::RngStreams
)


target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    /usr/local/include
)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
    CXX_STANDARD 20
)

install(
    TARGETS
    ${PROJECT_NAME}
    DESTINATION
    ${CMAKE_SOURCE_DIR}/bin
)

target_compile_options(
    ${PROJECT_NAME}
    PRIVATE
    -g -Wall
)

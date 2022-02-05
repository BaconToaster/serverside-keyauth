cmake_minimum_required(VERSION 3.19)
project(server)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -fpermissive")

add_executable(
    server
    server.cpp
)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

find_package(Threads REQUIRED)
find_package(CURL REQUIRED)

target_link_libraries(server Threads::Threads)
target_link_libraries(server curl)
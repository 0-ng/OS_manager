cmake_minimum_required(VERSION 3.19)
project(composity_2)

set(CMAKE_CXX_STANDARD 14)

#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads REQUIRED)

add_executable(composity_2 main.cpp)

find_package(Threads REQUIRED)
target_link_libraries(composity_2 Threads::Threads)


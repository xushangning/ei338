cmake_minimum_required(VERSION 3.15)
project(2 C)

set(CMAKE_C_STANDARD 11)

add_executable(2 main.c buffer.h producer_consumer.c)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(2 Threads::Threads)

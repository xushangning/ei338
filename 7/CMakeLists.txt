cmake_minimum_required(VERSION 3.15)
project(allocator C)

set(CMAKE_C_STANDARD 11)

add_executable(allocator main.c pdeque.c pdeque.h)

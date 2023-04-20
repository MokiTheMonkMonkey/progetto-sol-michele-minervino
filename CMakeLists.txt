cmake_minimum_required(VERSION 3.24)
project(untitled C)

set(CMAKE_C_STANDARD 99)

add_executable(untitled filesC/main.c includes/util.h filesC/utils.c filesC/bst.c includes/bst.h filesC/collector.c filesC/ThreadsPool.c includes/threadsPool.h)

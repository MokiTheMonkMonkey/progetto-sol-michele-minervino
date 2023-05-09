cmake_minimum_required(VERSION 3.24)
project(untitled C)

set(CMAKE_C_STANDARD 99)

add_executable(untitled source/main.c includes/util.h source/utils.c source/bst.c includes/bst.h source/collector.c source/ThreadsPool.c includes/threadsPool.h source/masterWorker.c includes/masterWorker.h)

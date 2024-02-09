cmake_minimum_required(VERSION 3.24)
project(pseudoFAT C)

set(CMAKE_C_STANDARD 99)

add_executable(pseudoFAT main.c functions.c functions.h
        iNodeFileSystem.c
        iNodeFileSystem.h
        bitmap_operations.c
        bitmap_operations.h
        directory_operations.c
        directory_operations.h)

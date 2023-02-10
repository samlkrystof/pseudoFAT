cmake_minimum_required(VERSION 3.24)
project(pseudoFAT C)

set(CMAKE_C_STANDARD 11)

add_executable(pseudoFAT main.c FATfileSystem.c FATfileSystem.h FATTable.c FATTable.h DirCluster.c DirCluster.h FileCluster.h sizes.h DirEntry.h functions.c functions.h)

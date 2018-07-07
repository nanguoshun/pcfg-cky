cmake_minimum_required(VERSION 3.10)
project(pcfg)

set(CMAKE_CXX_STANDARD 11)

add_executable(pcfg main.cpp common.h node.h node.cpp binarytree.h binarytree.cpp pcfg.h pcfg.cpp decoding.h docoding.cpp)
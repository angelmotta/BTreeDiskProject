cmake_minimum_required(VERSION 3.17)
project(btreeDisk)

set(CMAKE_CXX_STANDARD 14)

add_executable(btreeDisk main.cpp BtreeIndex.h Page.h)
cmake_minimum_required(VERSION 3.27.0)

project(SP_HW1 C)

add_compile_options(-Wall -Wextra -Wpedantic -g)

add_executable(ser ser.c)

add_executable(des des.c)

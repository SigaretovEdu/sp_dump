CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(sol)

set(CMAKE_C_COMPILER /usr/bin/x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/x86_64-w64-mingw32-g++)

ADD_COMPILE_OPTIONS(-Wall -Wextra -Wpedantic -g)

ADD_EXECUTABLE(parent parent.c)
ADD_EXECUTABLE(child child.c)

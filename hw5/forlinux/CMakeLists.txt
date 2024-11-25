CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(sol)

ADD_COMPILE_OPTIONS(-Wall -Wextra -Wpedantic -g)

ADD_EXECUTABLE(parent parent.c)
ADD_EXECUTABLE(child child.c)

# ADD_EXECUTABLE(testc testc.c)
# ADD_EXECUTABLE(testp testp.c)

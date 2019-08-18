MESSAGE(STATUS "working on libary dir")

FILE (GLOB_RECURSE libaryInt_SRCS "*.cpp" "*.cxx" "*.cc" "*.c")

set(libary_SRCS ${libaryInt_SRCS} PARENT_SCOPE)
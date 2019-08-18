MESSAGE(STATUS "working on application dir")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    SET (project_BIN ${PROJECT_NAME})
else()
    SET (project_BIN ${PROJECT_NAME}.bin)
endif()

set(project_SRCS main.cpp ${libary_SRCS})

add_executable(${project_BIN} ${project_SRCS})

#-L/opt/lib -lmedia
#Todo let CMake find the lib
TARGET_LINK_LIBRARIES(${project_BIN} /opt/lib/libmediaclient.so)
TARGET_LINK_LIBRARIES(${project_BIN} /opt/lib/libmcsimple.so)

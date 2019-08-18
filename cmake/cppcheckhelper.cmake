find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)

add_custom_target(
        cppcheck ALL
        COMMAND ${CMAKE_CXX_CPPCHECK}
        --enable=warning,performance,portability,information,missingInclude
        --std=c++11
        --project=compile_commands.json
        --template=gcc
        --quiet
        --library=../Tools/cppcheck/sdl.cfg
        -DELPP_CXX11
        --suppress-xml=${CMAKE_SOURCE_DIR}/CppCheckSuppressions.xml
)
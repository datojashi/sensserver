project(sensserver)
cmake_minimum_required(VERSION 3.2.0)
add_executable(sensserver main.cpp )

if(CMAKE_COMPILER_IS_GNUCXX)
   execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
   if (GCC_VERSION VERSION_GREATER 7.1 OR GCC_VERSION VERSION_EQUAL 7.1)
        message(STATUS "C++17 ENABLED.")
        set_property(GLOBAL PROPERTY CXX_STANDARD 17)
        add_definitions(-std=c++17)
        #add_compile_definitions("__STD_FILESYSTEM__")
    else()
        message(FATAL_ERROR "No appropriate compiler. Project should compiled with C++11 or greater. compiler GCC version > 4.3.")
    endif()
endif(CMAKE_COMPILER_IS_GNUCXX)

cmake_policy(SET CMP0076 NEW)
target_sources(sensserver
    PUBLIC
        sensorthread.cpp
        streamthread.cpp
        audiothread.cpp
        sensorthread.h
        streamthread.h
        audiothread.h
        senstelnet.cpp
        senstelnet.h
        )
target_include_directories(sensserver PRIVATE ${CMAKE_SOURCE_DIR}, ../AWL)
target_link_libraries(sensserver pthread)

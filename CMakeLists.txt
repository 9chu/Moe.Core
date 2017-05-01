cmake_minimum_required(VERSION 2.8)
project(MoeCore)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-g -Wall -Wextra")
set(CMAKE_C_FLAGS "-g -Wall -Wextra")

include_directories(include)
file(GLOB_RECURSE MOE_CORE_SRC src/*.cpp)

add_library(MoeCore STATIC ${MOE_CORE_SRC})

if(MOE_ENABLE_TEST)
    enable_testing()
    
    find_package(GTest REQUIRED)
    find_package(Threads REQUIRED)
    
    file(GLOB_RECURSE MOE_CORE_TEST_SRC Test/*.cpp)
    
    include_directories(${GTEST_INCLUDE_DIRS})

    add_executable(MoeCoreTest ${MOE_CORE_TEST_SRC})
    target_link_libraries(MoeCoreTest MoeCore ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    add_test(MoeCoreTest MoeCoreTest)
endif()

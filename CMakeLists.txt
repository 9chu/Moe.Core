cmake_minimum_required(VERSION 3.1)
project(MoeCore)

# 对外导出目录
set(MoeCore_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE STRING "Include-dir for MoeCore." FORCE)

# 编译选项
set(CMAKE_CXX_STANDARD 11)

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0600 -D_GNU_SOURCE -D_CRT_SECURE_NO_WARNINGS)
else()
    set(CMAKE_CXX_FLAGS "-g -Wall -Wextra")
    set(CMAKE_C_FLAGS "-g -Wall -Wextra")
endif()

# 工程包含目录
include_directories(include)
file(GLOB_RECURSE MOE_CORE_INCLUDE include/*.hpp)  # 无助于编译，仅帮助IDE产生代码提示
file(GLOB_RECURSE MOE_CORE_SRC src/*.cpp)

add_library(MoeCore STATIC ${MOE_CORE_SRC} ${MOE_CORE_INCLUDE})

if(MOE_ENABLE_TEST)
    enable_testing()
    
    find_package(GTest REQUIRED)
    find_package(Threads REQUIRED)
    
    file(GLOB_RECURSE MOE_CORE_TEST_SRC test/*.cpp)
    
    include_directories(${GTEST_INCLUDE_DIRS})

    add_executable(MoeCoreTest ${MOE_CORE_TEST_SRC})
    target_link_libraries(MoeCoreTest MoeCore ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    add_test(MoeCoreTest MoeCoreTest)
endif()

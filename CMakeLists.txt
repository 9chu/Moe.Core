cmake_minimum_required(VERSION 3.1)
project(MoeCore)

# 编译选项
set(CMAKE_CXX_STANDARD 11)

if(MSVC)
    add_definitions(-D_WIN32_WINNT=0x0600 -D_GNU_SOURCE -D_CRT_SECURE_NO_WARNINGS)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -Wno-implicit-fallthrough")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra -Wno-implicit-fallthrough")
endif()

# 目标
file(GLOB_RECURSE MOE_CORE_SRC src/*.cpp include/*.hpp)

add_library(MoeCore STATIC ${MOE_CORE_SRC})
target_include_directories(MoeCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# 单元测试
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

cmake_minimum_required(VERSION 2.8)
project(TiStore)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message("------------ Options -------------")
message("  CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

message("----------------------------------")

message("-------------- Env ---------------")
message("  CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message("  CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message("  PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message("  PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")
message("----------------------------------")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG")
endif()

if (NOT MSVC)
    ## -Wall -Werror -Wextra -Wno-format -Wno-unused-function
    set(CMAKE_CXX_FLAGS_DEFAULT "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-variable -fPIC")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEFAULT} -O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEFAULT} -g -pg -D_DEBUG")
endif()

if (WIN32)
    ADD_COMPILE_OPTIONS("-D_WIN32_WINNT=0x0601")
    ADD_COMPILE_OPTIONS("-D_CRT_SECURE_NO_WARNINGS")
    SET(EXTRA_LIBS ${EXTRA_LIBS} ws2_32 mswsock)
endif()

if (UNIX)
    set(EXTRA_LIBS ${EXTRA_LIBS} pthread)
else()
    set(EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif()

include_directories(./)

set(SOURCE_FILES
    TiStore/basic/ssize.h
    TiStore/basic/stdint.h
    TiStore/fs/BlockDevice.h
    TiStore/fs/Common.h
    TiStore/fs/ErrorCode.h
    TiStore/fs/FileSystem.h
    TiStore/fs/Initor.h
    TiStore/fs/MetaData.h
    TiStore/fs/SuperBlock.h
    TiStore/kv/BloomFilter.h
    TiStore/kv/BloomFilterFixed.h
    TiStore/kv/Hash.h
    TiStore/kv/SkipList.h
    TiStore/kv/Slice.h
    TiStore/lang/Property.h
    TiStore/lang/TypeInfo.h
    TiStore/traits/bool_type.h
    TiStore/traits/common.h
    TiStore/traits/const_type.h
    TiStore/traits/integral_constant.h
    TiStore/traits/is_same.h
    TiStore/traits/remove_const.h
    TiStore/traits/remove_cv.h
    TiStore/traits/remove_volatile.h
    TiStore/TiFS.cpp
    TiStore/TiFS.h
    TiStore/TiStore.h
    TiStore/traits.h
    TiStoreTest/stop_watch.h
    TiStoreTest/test.cpp
    TiStoreTest/test.h
    TiStoreTest/TiStoreTest.cpp)

add_executable(TiStore ${SOURCE_FILES})
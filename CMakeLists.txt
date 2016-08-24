CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(TiStore)

SET(CMAKE_VERBOSE_MAKEFILE ON)

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

IF (NOT MSVC)
    ## -Wall -Werror -Wextra -Wno-format -Wno-unused-function
    set(CMAKE_CXX_FLAGS_DEFAULT "-std=c++11 -Wall -Wno-unused-function -fPIC -pthread")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEFAULT} -O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEFAULT} -g -pg -D_DEBUG")
    ##ADD_COMPILE_OPTIONS("-std=c++11")
    ##ADD_COMPILE_OPTIONS("-Wall")
    ##ADD_COMPILE_OPTIONS("-Wno-unused-function")
ENDIF()

IF (WIN32)
    ADD_COMPILE_OPTIONS("-D_WIN32_WINNT=0x0501")
    ADD_COMPILE_OPTIONS("-D_CRT_SECURE_NO_WARNINGS")
    SET(EXTRA_LIBS ${EXTRA_LIBS} ws2_32 mswsock)
ENDIF()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG")
endif()

SET(EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT})

IF (UNIX)
    SET(EXTRA_LIBS ${EXTRA_LIBS} pthread)
ENDIF()

include_directories(src)

set(SOURCE_FILES 
    src/TiStoreTest/TiStoreTest.cpp
    src/TiStoreTest/test.cpp
    )

ADD_CUSTOM_TARGET(debug
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
    COMMENT "Switch CMAKE_BUILD_TYPE to Debug"

)

ADD_CUSTOM_TARGET(release
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
    COMMENT "Switch CMAKE_BUILD_TYPE to Release"
)

add_executable(TiStore ${SOURCE_FILES})
target_link_libraries(TiStore ${EXTRA_LIBS})

# Copyright (c) Benjamin Kietzman (github.com/bkietz)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

project(boost-dbus CXX)

###############
# CMake options
cmake_minimum_required(VERSION 2.8)

###############
# C++ options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")#-std=c++0x")
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/test)

###############
# import Boost
find_package(Boost 1.54 COMPONENTS system chrono REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

###############
# import D-Bus
find_package(PkgConfig REQUIRED)
pkg_check_modules(DBus dbus-1)
include_directories(${DBus_INCLUDE_DIRS})
link_directories(${DBus_LIBRARY_DIRS})

##############
# import GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

find_package(Threads REQUIRED)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  message("Use clang")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
endif()

##############
# Tests
enable_testing()

cmake_policy(SET CMP0023 NEW)
function(cxx_test test_name)
  add_executable(${test_name} "test/${test_name}.cpp")
  target_link_libraries(${test_name} PRIVATE ${Boost_LIBRARIES})
  target_link_libraries(${test_name} PRIVATE ${DBus_LIBRARIES})
  target_link_libraries(${test_name} PRIVATE ${GTEST_BOTH_LIBRARIES})
  target_link_libraries(${test_name} PRIVATE Threads::Threads)
  add_test(${test_name} ${test_name} "--gtest_output=xml:${test_name}.xml")
endfunction(cxx_test)

cxx_test(avahi)
cxx_test(message)
# cxx_test(demo)

add_executable(demo "test/demo.cpp")
target_link_libraries(demo PRIVATE ${Boost_LIBRARIES})
target_link_libraries(demo PRIVATE ${DBus_LIBRARIES})
target_link_libraries(demo PRIVATE Threads::Threads)
# target_link_libraries(demo PRIVATE ${GTEST_BOTH_LIBRARIES})

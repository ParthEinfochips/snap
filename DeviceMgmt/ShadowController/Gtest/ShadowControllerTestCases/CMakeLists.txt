cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(CMAKE_CXX_FLAGS "-std=c++1y -g")

add_executable(SCSetDevTest SCSetDevice.cc)
target_link_libraries(SCSetDevTest gtest gtest_main alljoyn)

#add_executable(testCase Gtest/SoftwareAgentTestCases/TestCase1.cc ${SRC_LIST})
#target_link_libraries(testCase pthread ftp boost_system boost_thread usb dl zip gtest gtest_main)


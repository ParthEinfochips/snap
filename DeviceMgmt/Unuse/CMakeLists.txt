project(DeviceMgmt)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(headers SRC_LIST)
aux_source_directory(AJAttachment SRC_LIST)
aux_source_directory(. SRC_LIST)

set(CMAKE_CXX_FLAGS "-std=c++1y -DROUTER -DQCC_OS_GROUP_POSIX -DQCC_OS_LINUX -L/home/einfochips/pi/HawkEdge_Git_17Feb/HawkEdge/HawkRouter/build/linux/x86_64/debug/dist/cpp/lib")

add_executable(${PROJECT_NAME} ${SRC_LIST})

include_directories(/home/einfochips/pi/HawkEdge_Git_17Feb/HawkEdge/HawkRouter/build/linux/x86_64/debug/dist/cpp/inc/)

target_link_libraries(DeviceMgmt pthread boost_system ajrouter alljoyn alljoyn_about dl)

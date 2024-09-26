cmake_minimum_required(VERSION 3.22)
project(ethercat-test)

add_subdirectory(libs/SOEM)

set(SOURCES main.cpp EthercatCOE.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC soem)

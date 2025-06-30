cmake_minimum_required(VERSION 3.20)

project(cppkg)

include_directories(include ${PROJECT_SOURCE_DIR}/third_party)

add_executable(${PROJECT_NAME} src/cppkg.cc)

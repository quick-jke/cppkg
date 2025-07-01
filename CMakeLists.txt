cmake_minimum_required(VERSION 3.20)

project(cppkg)

include_directories(include ${PROJECT_SOURCE_DIR}/third_party)

add_executable(${PROJECT_NAME} src/cppkg.cc)

install(
    TARGETS ${PROJECT_NAME}
    DESTINATION bin
)

install(
    DIRECTORY include/
    DESTINATION include
)


if(EXISTS "${PROJECT_SOURCE_DIR}/share")
    install(
        DIRECTORY share/
        DESTINATION share
    )
endif()


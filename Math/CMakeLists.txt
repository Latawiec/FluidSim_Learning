cmake_minimum_required(VERSION 3.9)

project(Math)

add_library(${PROJECT_NAME} INTERFACE
)

add_dependencies(${PROJECT_NAME}
    Marcin2D
)

target_link_libraries(${PROJECT_NAME}
INTERFACE
    Marcin2D
)

target_include_directories(${PROJECT_NAME} INTERFACE
    include
)
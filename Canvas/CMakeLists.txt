cmake_minimum_required(VERSION 3.9)

project(Canvas)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

add_library(${PROJECT_NAME}
    Canvas.cpp
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    OpenGL::GL
    glfw
    glad::glad
)

target_include_directories(${PROJECT_NAME} PUBLIC
    .
)
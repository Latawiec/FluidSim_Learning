cmake_minimum_required(VERSION 3.9)

project(JosStamFluidSimulation)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
)

add_dependencies(${PROJECT_NAME}
    Marcin2D
    Math
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    Marcin2D
    Math
    OpenGL::GL
    glfw
    glad::glad
    glm
)
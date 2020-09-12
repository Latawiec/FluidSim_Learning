cmake_minimum_required(VERSION 3.9)

project(JosStamFluidSimulation)

find_package(glm CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    #Jos_copypaste.cpp
)

add_dependencies(${PROJECT_NAME}
    Marcin2D
    Math
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    Marcin2D
    Canvas
    Math
    glm
)
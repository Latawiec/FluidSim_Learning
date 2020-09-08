cmake_minimum_required(VERSION 3.9)

project(Marcin2D)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE
    .
)
cmake_minimum_required(VERSION 3.9)

# Install - set to false if you don't want autoinstall
set(AUTOINSTALL true)

# Check
if (NOT DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message (WARNING "Neither VCPKG_ROOT nor CMAKE_TOOLCHAIN_FILE specified. We might not be able to make it!" )
endif()

# Setup
if (DEFINED ENV{VCPKG_ROOT})
    if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "")
    endif()
endif()

if (AUTOINSTALL)
    if (WIN32) 
        set(VCPKG_EXE $ENV{VCPKG_ROOT}/vcpkg.exe)
    else()
        set(VCPKG_EXE $ENV{VCPKG_ROOT}/vcpkg)
    endif()
    # Read requirements
    file(READ vcpkg_required.txt VCPKG_REQUIRED NEWLINE_CONSUME)
    string(REPLACE "\n" "; " VCPKG_REQUIRED ${VCPKG_REQUIRED})
    string(REPLACE " " ";" VCPKG_REQUIRED ${VCPKG_REQUIRED})
    execute_process(COMMAND "${VCPKG_EXE}" "install" ${VCPKG_REQUIRED})
endif()

project(Fluids)

set(CMAKE_CXX_STANDARD 17) 

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3")
set(CMAKE_GENERATOR_PLATFORM x64)


add_subdirectory(Marcin2D)
add_subdirectory(Math)
add_subdirectory(Canvas)
add_subdirectory(JosStamFluidSimulation)
cmake_minimum_required(VERSION 3.16)
project(Battleship LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BATTLESHIP_ENABLE_WARNINGS "Enable compiler warnings" ON)
option(BATTLESHIP_FETCH_SFML "Automatically fetch SFML if not found" ON)

if(BATTLESHIP_FETCH_SFML)
    find_package(SFML 2.5 COMPONENTS graphics window system QUIET)
    if(NOT SFML_FOUND)
        if(UNIX AND NOT APPLE)
            find_package(OpenGL QUIET)
            if(NOT OpenGL_FOUND)
                message(FATAL_ERROR
                    "SFML auto-fetch requires OpenGL development files on Linux. "
                    "Install them (for example: libgl1-mesa-dev and xorg-dev) "
                    "and re-run CMake."
                )
            endif()
        endif()

        include(FetchContent)
        message(STATUS "SFML not found locally. Fetching SFML from source...")

        set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
        set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
        set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
        set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)

        FetchContent_Declare(
            sfml
            GIT_REPOSITORY https://github.com/SFML/SFML.git
            GIT_TAG 2.6.2
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(sfml)
    endif()
else()
    find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
endif()

add_executable(Battleship
    main.cpp
    Arsenal.cpp
    Game.cpp
    Ship.cpp
    FlyingPlane.cpp
    Effect.cpp
)

target_include_directories(Battleship PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(BATTLESHIP_ENABLE_WARNINGS)
    if(MSVC)
        target_compile_options(Battleship PRIVATE /W4)
    else()
        target_compile_options(Battleship PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

if(TARGET sfml-graphics)
    target_link_libraries(Battleship PRIVATE sfml-graphics sfml-window sfml-system)
elseif(TARGET SFML::Graphics)
    target_link_libraries(Battleship PRIVATE SFML::Graphics SFML::Window SFML::System)
else()
    message(FATAL_ERROR "SFML targets were not found after dependency resolution")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
    add_custom_command(TARGET Battleship POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:Battleship>/assets/
    )
endif()

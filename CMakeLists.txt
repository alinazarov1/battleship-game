cmake_minimum_required(VERSION 3.16)
project(Battleship LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BATTLESHIP_ENABLE_WARNINGS "Enable compiler warnings" ON)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

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

target_link_libraries(Battleship PRIVATE sfml-graphics sfml-window sfml-system)

if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
    add_custom_command(TARGET Battleship POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:Battleship>/assets/
    )
endif()

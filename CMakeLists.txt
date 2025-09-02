cmake_minimum_required(VERSION 3.10)
project(Battleship)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_definitions(-DSFML_STATIC -DGLEW_STATIC)


set(SFML_DIR "${CMAKE_SOURCE_DIR}/sfml")
include_directories(
    ${SFML_DIR}/include
    ${SFML_DIR}/include/glm
    ${SFML_DIR}/include/GLFW
    ${SFML_DIR}/include/glew
)

link_directories(${SFML_DIR}/lib)

add_executable(Battleship
    main.cpp
    Arsenal.h
    Game.h   
    FlyingPlane.h 
    Common.h
    Ship.h 
    Effect.h
    Arsenal.cpp
    Game.cpp
    Ship.cpp
    FlyingPlane.cpp
    Effect.cpp)
add_custom_command(TARGET Battleship POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:Battleship>/assets/
)
target_link_libraries(Battleship
    sfml-graphics-s-d
    sfml-window-s-d
    sfml-system-s-d
    glew32s
    glfw3
    opengl32
    freetype
    winmm
    gdi32
)

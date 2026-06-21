cmake_minimum_required(VERSION 3.22)

project(MyEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(MyEngine
    src/main.cpp
    src/Triangle.h
    src/Triangle.cpp
    src/Camera.h
    src/Camera.cpp
)

target_link_libraries(MyEngine
    PRIVATE
        glfw
        glad::glad
        glm::glm
)

target_compile_features(MyEngine
    PRIVATE
        cxx_std_20
)
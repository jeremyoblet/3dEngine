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
    src/Viewer.cpp
    src/Objects/Cameras/Camera.h
    src/Objects/Cameras/Camera.cpp
    src/Objects/Lights/PointLight.h
    src/Objects/Primitives/Triangle.h
    src/Objects/Primitives/Triangle.cpp
    src/Objects/Primitives/Cube.h
    src/Objects/Primitives/Cube.cpp
    src/Objects/Display/Grid.h
    src/Objects/Display/Grid.cpp
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
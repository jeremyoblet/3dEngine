cmake_minimum_required(VERSION 3.22)

project(MyEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGLWidgets)

qt_add_executable(MyEngine
    src/main.cpp

    # UI
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/ui/MenuContext.h
    src/ui/MenuContext.cpp

    # Outliner
    src/ui/Outliner/SceneModel.h
    src/ui/Outliner/SceneModel.cpp
    src/ui/Outliner/OutlinerWidget.h
    src/ui/Outliner/OutlinerWidget.cpp

    # Viewers
    src/ui/Viewers/3d/ViewerWidget3D.h
    src/ui/Viewers/3d/ViewerWidget3D.cpp

    # Core
    src/Core/Transform.h
    src/Core/Transform.cpp
    src/Core/Shape.h
    src/Core/Material.h
    src/Core/Material.cpp
    src/Core/Mesh.h
    src/Core/Mesh.cpp
    src/Core/DrawContext.h
    src/Core/Object.h
    src/Core/Object.cpp
    src/Core/Scene.h
    src/Core/Scene.cpp

    # Cameras
    src/Objects/Cameras/Camera.h
    src/Objects/Cameras/Camera.cpp

    # Lights
    src/Objects/Lights/PointLight.h
    src/Objects/Lights/PointLightObject.h
    src/Objects/Lights/PointLightObject.cpp

    # Primitives
    src/Objects/Primitives/Triangle.h
    src/Objects/Primitives/Triangle.cpp
    src/Objects/Primitives/Cube.h
    src/Objects/Primitives/Cube.cpp
    src/Objects/Primitives/Sphere.h
    src/Objects/Primitives/Sphere.cpp

    # Display
    src/Objects/Display/Grid.h
    src/Objects/Display/Grid.cpp
    src/Objects/Display/Gizmo/GizmoTranslation.h
    src/Objects/Display/Gizmo/GizmoTranslation.cpp
    src/Objects/Display/Gizmo/GizmoRotation.h
    src/Objects/Display/Gizmo/GizmoRotation.cpp
    src/Objects/Display/Gizmo/GizmoScale.h
    src/Objects/Display/Gizmo/GizmoScale.cpp
)

target_include_directories(MyEngine PRIVATE src)

target_link_libraries(MyEngine
    PRIVATE
        glad::glad
        glm::glm
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::OpenGLWidgets
)

target_compile_features(MyEngine
    PRIVATE
        cxx_std_20
)

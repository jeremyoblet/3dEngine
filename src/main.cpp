#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Camera.h"
#include "Triangle.h"

namespace
{
    constexpr int WINDOW_WIDTH  = 1280;
    constexpr int WINDOW_HEIGHT = 720;

    Camera camera;
    bool   leftDown   = false;
    bool   middleDown = false;
    double lastX      = 0.0;
    double lastY      = 0.0;
}

void mouseButtonCallback(GLFWwindow*, int btn, int action, int)
{
    leftDown   = (btn == GLFW_MOUSE_BUTTON_LEFT   && action == GLFW_PRESS);
    middleDown = (btn == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS);
}

void cursorCallback(GLFWwindow*, double x, double y)
{
    const float dx = static_cast<float>(x - lastX);
    const float dy = static_cast<float>(y - lastY);

    if (leftDown)   camera.orbit(dx, dy);
    if (middleDown) camera.pan(dx, dy);

    lastX = x;
    lastY = y;
}

void scrollCallback(GLFWwindow*, double, double dy)
{
    camera.zoom(static_cast<float>(dy));
}


int main()
{
    if (glfwInit() == GLFW_FALSE)
    {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* p_window = glfwCreateWindow(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        "MyEngine",
        nullptr,
        nullptr);

    if (p_window == nullptr)
    {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(p_window);

    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0)
    {
        std::cerr << "Failed to initialize GLAD." << std::endl;
        glfwDestroyWindow(p_window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    // Callbacks
    glfwSetMouseButtonCallback(p_window, mouseButtonCallback);
    glfwSetCursorPosCallback  (p_window, cursorCallback);
    glfwSetScrollCallback     (p_window, scrollCallback);

    // Objets (après GLAD)
    Triangle triangle;

    glEnable(GL_DEPTH_TEST);

    while (glfwWindowShouldClose(p_window) == GLFW_FALSE)
    {
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glClearColor(0.1F, 0.1F, 0.1F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Matrices MVP
        const float aspect = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
        const glm::mat4 model = glm::mat4(1.0f);
        const glm::mat4 view  = camera.getView();
        const glm::mat4 proj  = camera.getProjection(aspect);
        const glm::mat4 MVP   = proj * view * model;

        triangle.draw(MVP);

        glfwSwapBuffers(p_window);
        glfwPollEvents();
    }

    glfwDestroyWindow(p_window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
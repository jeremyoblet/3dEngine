#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Objects/Cameras/Camera.h"
#include "Objects/Lights/PointLight.h"
#include "Objects/Primitives/Cube.h"
#include "Objects/Display/Grid.h"
#include "Objects/Display/Gizmo.h"

namespace
{
    constexpr int WINDOW_WIDTH  = 1280;
    constexpr int WINDOW_HEIGHT = 720;

    Camera     camera;
    PointLight light;

    // Position du cube dans la scène
    glm::vec3  cubePos       = { 0.0f, 0.0f, 0.0f };

    // Matrices courantes (mises à jour chaque frame, lues par les callbacks)
    glm::mat4  g_view(1.0f);
    glm::mat4  g_proj(1.0f);
    float      gizmoScale = 1.0f;

    // État souris
    bool   leftDown    = false;
    bool   middleDown  = false;
    double lastX       = 0.0;
    double lastY       = 0.0;
    int    dragAxis    = -1;   // -1 = aucun, 0/1/2 = X/Y/Z
}

// ---- Helpers gizmo ---------------------------------------------------------

// Projette un point 3D en coordonnées écran (pixels, Y vers le bas)
static glm::vec2 worldToScreen(glm::vec3 p)
{
    glm::vec4 clip = g_proj * g_view * glm::vec4(p, 1.0f);
    glm::vec3 ndc  = glm::vec3(clip) / clip.w;
    return { (ndc.x + 1.0f) * 0.5f * WINDOW_WIDTH,
             (1.0f - ndc.y) * 0.5f * WINDOW_HEIGHT };
}

// Direction écran de l'axe i (en pixels pour 1 unité monde)
static glm::vec2 screenAxisDir(int axis)
{
    static constexpr glm::vec3 dirs[3] = { {1,0,0}, {0,1,0}, {0,0,1} };
    return worldToScreen(cubePos + dirs[axis]) - worldToScreen(cubePos);
}

// Retourne l'axe du gizmo le plus proche de la souris (-1 si aucun)
static int gizmoHitTest(double mx, double my)
{
    static constexpr glm::vec3 dirs[3] = { {1,0,0}, {0,1,0}, {0,0,1} };
    const glm::vec2 mouse  = { (float)mx, (float)my };
    const glm::vec2 origin = worldToScreen(cubePos);
    constexpr float THRESH = 12.0f; // pixels

    int   best = -1;
    float minD = THRESH;

    for (int i = 0; i < 3; ++i) {
        glm::vec2 tip = worldToScreen(cubePos + dirs[i] * gizmoScale);
        glm::vec2 seg = tip - origin;
        float segLen2 = glm::dot(seg, seg);
        if (segLen2 < 1.0f) continue;

        float t = glm::clamp(glm::dot(mouse - origin, seg) / segLen2, 0.0f, 1.0f);
        float d = glm::length(mouse - (origin + t * seg));
        if (d < minD) { minD = d; best = i; }
    }
    return best;
}

// ---- Callbacks -------------------------------------------------------------

void mouseButtonCallback(GLFWwindow* w, int btn, int action, int)
{
    if (btn == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            double mx, my;
            glfwGetCursorPos(w, &mx, &my);
            dragAxis = gizmoHitTest(mx, my);
            leftDown = (dragAxis < 0);
        } else {
            leftDown = false;
            dragAxis = -1;
        }
    }
    if (btn == GLFW_MOUSE_BUTTON_MIDDLE)
        middleDown = (action == GLFW_PRESS);
}

void cursorCallback(GLFWwindow*, double x, double y)
{
    const float dx = static_cast<float>(x - lastX);
    const float dy = static_cast<float>(y - lastY);

    if (dragAxis >= 0) {
        // Déplacement du cube le long de l'axe sélectionné
        glm::vec2 sa  = screenAxisDir(dragAxis);
        float     len = glm::length(sa);
        if (len > 1.0f) {
            static constexpr glm::vec3 dirs[3] = { {1,0,0}, {0,1,0}, {0,0,1} };
            float proj      = (dx * sa.x + dy * sa.y) / len; // pixels projetés
            float worldDelta = proj / len;                     // 1 unité = len pixels
            cubePos += dirs[dragAxis] * worldDelta;
        }
    } else {
        if (leftDown)   camera.orbit(dx, dy);
        if (middleDown) camera.pan(dx, dy);
    }

    lastX = x;
    lastY = y;
}

void scrollCallback(GLFWwindow*, double, double dy)
{
    camera.zoom(static_cast<float>(dy));
}

// ---- Main ------------------------------------------------------------------

int main()
{
    if (glfwInit() == GLFW_FALSE) {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* p_window = glfwCreateWindow(
        WINDOW_WIDTH, WINDOW_HEIGHT, "MyEngine", nullptr, nullptr);

    if (!p_window) {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(p_window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize GLAD." << std::endl;
        glfwDestroyWindow(p_window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    glfwSetMouseButtonCallback(p_window, mouseButtonCallback);
    glfwSetCursorPosCallback  (p_window, cursorCallback);
    glfwSetScrollCallback     (p_window, scrollCallback);

    // Objets (après GLAD)
    Cube  cube;
    Grid  grid;
    Gizmo gizmo;

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(p_window))
    {
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const float aspect = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
        g_view = camera.getView();
        g_proj = camera.getProjection(aspect);

        // Le gizmo garde une taille visuelle constante quelle que soit la distance
        gizmoScale = glm::length(camera.getPosition() - cubePos) * 0.18f;

        const glm::mat4 model = glm::translate(glm::mat4(1.0f), cubePos);
        const glm::mat4 MVP   = g_proj * g_view * model;

        grid.draw(MVP);
        cube.draw(MVP, model, light, camera.getPosition());
        gizmo.draw(g_view, g_proj, cubePos, gizmoScale);

        glfwSwapBuffers(p_window);
        glfwPollEvents();
    }

    glfwDestroyWindow(p_window);
    glfwTerminate();
    return EXIT_SUCCESS;
}

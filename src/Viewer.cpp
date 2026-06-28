#include <iostream>
#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Core/DrawContext.h"
#include "Objects/Cameras/Camera.h"
#include "Objects/Lights/PointLight.h"
#include "Objects/Primitives/Cube.h"
#include "Objects/Display/Grid.h"
#include "Objects/Display/Gizmo/GizmoTranslation.h"
#include "Objects/Display/Gizmo/GizmoRotation.h"
#include "Objects/Display/Gizmo/GizmoScale.h"

namespace
{
    constexpr int WINDOW_WIDTH  = 1280;
    constexpr int WINDOW_HEIGHT = 720;

    Camera     camera;
    PointLight light;

    Cube*  g_cube  = nullptr;   // initialisé après GLAD
    float  gizmoVisualScale = 1.0f;

    // Matrices courantes (lues par les callbacks)
    glm::mat4 g_view(1.0f);
    glm::mat4 g_proj(1.0f);

    // État souris
    bool   leftDown   = false;
    bool   middleDown = false;
    double lastX      = 0.0;
    double lastY      = 0.0;
    int    dragAxis         = -1;  // axe de translation actif (-1 = aucun)
    int    rotationDragAxis = -1;  // axe de rotation actif    (-1 = aucun)
    int    scaleDragAxis    = -1;  // axe de scale actif       (-1 = aucun)

    enum class GizmoMode { Translation, Rotation, Scale };
    GizmoMode activeGizmo = GizmoMode::Translation;
}

// ---- Helpers ---------------------------------------------------------------

// Retourne la direction du rayon caméra en espace monde pour la position souris (mx, my)
static glm::vec3 getCameraRay(double mx, double my)
{
    float nx = 2.0f * (float)mx / WINDOW_WIDTH  - 1.0f;
    float ny = 1.0f - 2.0f * (float)my / WINDOW_HEIGHT;
    glm::mat4 invPV = glm::inverse(g_proj * g_view);
    glm::vec4 near4 = invPV * glm::vec4(nx, ny, -1.0f, 1.0f);
    near4 /= near4.w;
    glm::vec4 far4  = invPV * glm::vec4(nx, ny,  1.0f, 1.0f);
    far4  /= far4.w;
    return glm::normalize(glm::vec3(far4) - glm::vec3(near4));
}

static bool rayHitsSphere(glm::vec3 ro, glm::vec3 rd, glm::vec3 center, float radius)
{
    glm::vec3 oc = ro - center;
    float b = glm::dot(oc, rd);
    float c = glm::dot(oc, oc) - radius * radius;
    return (b * b - c) >= 0.0f;
}

// ---- Helpers gizmo ---------------------------------------------------------

static glm::vec2 worldToScreen(glm::vec3 p)
{
    glm::vec4 clip = g_proj * g_view * glm::vec4(p, 1.0f);
    glm::vec3 ndc  = glm::vec3(clip) / clip.w;
    return { (ndc.x + 1.0f) * 0.5f * WINDOW_WIDTH,
             (1.0f - ndc.y) * 0.5f * WINDOW_HEIGHT };
}

static glm::vec2 screenAxisDir(int axis)
{
    static constexpr glm::vec3 dirs[3] = { {1,0,0}, {0,1,0}, {0,0,1} };
    const glm::vec3& pos = g_cube->transform.position;
    return worldToScreen(pos + dirs[axis]) - worldToScreen(pos);
}

static int gizmoHitTest(double mx, double my)
{
    static constexpr glm::vec3 dirs[3] = { {1,0,0}, {0,1,0}, {0,0,1} };
    const glm::vec3& pos   = g_cube->transform.position;
    const glm::vec2  mouse = { (float)mx, (float)my };
    const glm::vec2  origin = worldToScreen(pos);
    constexpr float  THRESH = 12.0f;

    int   best = -1;
    float minD = THRESH;

    for (int i = 0; i < 3; ++i) {
        glm::vec2 tip = worldToScreen(pos + dirs[i] * gizmoVisualScale);
        glm::vec2 seg = tip - origin;
        float segLen2 = glm::dot(seg, seg);
        if (segLen2 < 1.0f) continue;

        float t = glm::clamp(glm::dot(mouse - origin, seg) / segLen2, 0.0f, 1.0f);
        float d = glm::length(mouse - (origin + t * seg));
        if (d < minD) { minD = d; best = i; }
    }
    return best;
}

// Retourne l'axe du gizmo de scale le plus proche de la souris (-1 si aucun)
static int scaleHitTest(double mx, double my)
{
    static constexpr glm::vec3 dirs[3] = { {1,0,0}, {0,1,0}, {0,0,1} };
    const glm::vec2  mouse  = { (float)mx, (float)my };
    const glm::vec3& pos    = g_cube->transform.position;
    const glm::vec2  origin = worldToScreen(pos);
    constexpr float  THRESH = 12.0f;

    int   best = -1;
    float minD = THRESH;

    for (int i = 0; i < 3; ++i) {
        glm::vec2 tip = worldToScreen(pos + dirs[i] * gizmoVisualScale);
        glm::vec2 seg = tip - origin;
        float segLen2 = glm::dot(seg, seg);
        if (segLen2 < 1.0f) continue;

        float t = glm::clamp(glm::dot(mouse - origin, seg) / segLen2, 0.0f, 1.0f);
        float d = glm::length(mouse - (origin + t * seg));
        if (d < minD) { minD = d; best = i; }
    }
    return best;
}

// Retourne l'axe de l'anneau de rotation le plus proche de la souris (-1 si aucun)
static int rotationHitTest(double mx, double my)
{
    const int   N      = 64;
    const float TWO_PI = 6.28318530718f;

    struct Ring { glm::vec3 u, v; };
    constexpr Ring rings[3] = {
        { {0,1,0}, {0,0,1} }, // anneau X
        { {1,0,0}, {0,0,1} }, // anneau Y
        { {1,0,0}, {0,1,0} }, // anneau Z
    };

    const glm::vec2  mouse  = { (float)mx, (float)my };
    const glm::vec3& pos    = g_cube->transform.position;
    constexpr float  THRESH = 10.0f;

    int   best = -1;
    float minD = THRESH;

    for (int ri = 0; ri < 3; ++ri) {
        for (int i = 0; i < N; ++i) {
            float a0 = TWO_PI * i       / N;
            float a1 = TWO_PI * (i + 1) / N;
            glm::vec3 p0 = pos + (std::cos(a0) * rings[ri].u + std::sin(a0) * rings[ri].v) * gizmoVisualScale;
            glm::vec3 p1 = pos + (std::cos(a1) * rings[ri].u + std::sin(a1) * rings[ri].v) * gizmoVisualScale;
            glm::vec2 s0 = worldToScreen(p0);
            glm::vec2 s1 = worldToScreen(p1);

            glm::vec2 seg  = s1 - s0;
            float segLen2  = glm::dot(seg, seg);
            if (segLen2 < 0.1f) continue;

            float t = glm::clamp(glm::dot(mouse - s0, seg) / segLen2, 0.0f, 1.0f);
            float d = glm::length(mouse - (s0 + t * seg));
            if (d < minD) { minD = d; best = ri; }
        }
    }
    return best;
}

// ---- Callbacks -------------------------------------------------------------

void keyCallback(GLFWwindow*, int key, int, int action, int)
{
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_W) activeGizmo = GizmoMode::Translation;
    if (key == GLFW_KEY_E) activeGizmo = GizmoMode::Rotation;
    if (key == GLFW_KEY_R) activeGizmo = GizmoMode::Scale;
    if (key == GLFW_KEY_Z && g_cube)
        g_cube->wireframe = !g_cube->wireframe;
    if (key == GLFW_KEY_F && g_cube && g_cube->selected) {
        const glm::vec3& s = g_cube->transform.scale;
        float size = std::max({ s.x, s.y, s.z });
        camera.focusOn(g_cube->transform.position, size);
    }
}

void mouseButtonCallback(GLFWwindow* w, int btn, int action, int)
{
    if (btn == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            double mx, my;
            glfwGetCursorPos(w, &mx, &my);

            bool gizmoConsumed = false;
            if (g_cube->selected) {
                switch (activeGizmo) {
                    case GizmoMode::Translation:
                        dragAxis = gizmoHitTest(mx, my);
                        gizmoConsumed = dragAxis >= 0;
                        break;
                    case GizmoMode::Rotation:
                        rotationDragAxis = rotationHitTest(mx, my);
                        gizmoConsumed = rotationDragAxis >= 0;
                        break;
                    case GizmoMode::Scale:
                        scaleDragAxis = scaleHitTest(mx, my);
                        gizmoConsumed = scaleDragAxis >= 0;
                        break;
                }
            }
            if (!gizmoConsumed) {
                // Test sélection (sphère englobante)
                const glm::vec3& s = g_cube->transform.scale;
                float radius = 0.866f * std::max({ s.x, s.y, s.z });
                glm::vec3 ro = camera.getPosition();
                glm::vec3 rd = getCameraRay(mx, my);
                if (rayHitsSphere(ro, rd, g_cube->transform.position, radius))
                    g_cube->selected = !g_cube->selected;
                else
                    leftDown = true; // vide → orbit
            }
        } else {
            leftDown         = false;
            dragAxis         = -1;
            rotationDragAxis = -1;
            scaleDragAxis    = -1;
        }
    }
    if (btn == GLFW_MOUSE_BUTTON_MIDDLE)
        middleDown = (action == GLFW_PRESS);
}

void cursorCallback(GLFWwindow*, double x, double y)
{
    const float dx = static_cast<float>(x - lastX);
    const float dy = static_cast<float>(y - lastY);

    static constexpr glm::vec3 axisDirs[3] = { {1,0,0}, {0,1,0}, {0,0,1} };

    if (dragAxis >= 0) {
        // Translation contrainte sur un axe
        glm::vec2 sa  = screenAxisDir(dragAxis);
        float     len = glm::length(sa);
        if (len > 1.0f) {
            float worldDelta = (dx * sa.x + dy * sa.y) / (len * len);
            g_cube->transform.position += axisDirs[dragAxis] * worldDelta;
        }
    } else if (rotationDragAxis >= 0) {
        // Rotation autour d'un axe
        const glm::vec3& pos = g_cube->transform.position;
        glm::vec2 sa  = worldToScreen(pos + axisDirs[rotationDragAxis]) - worldToScreen(pos);
        float     len = glm::length(sa);
        if (len > 1.0f) {
            glm::vec2 perp = glm::vec2(-sa.y, sa.x) / len;
            float degrees  = (dx * perp.x + dy * perp.y) * 0.5f;
            g_cube->transform.rotation[rotationDragAxis] += degrees;
        }
    } else if (scaleDragAxis >= 0) {
        // Scale le long d'un axe
        glm::vec2 sa  = screenAxisDir(scaleDragAxis);
        float     len = glm::length(sa);
        if (len > 1.0f) {
            float delta = (dx * sa.x + dy * sa.y) / (len * len);
            float& s    = g_cube->transform.scale[scaleDragAxis];
            s = std::max(0.01f, s + delta * 1.5f);
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
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

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

    glfwSetKeyCallback        (p_window, keyCallback);
    glfwSetMouseButtonCallback(p_window, mouseButtonCallback);
    glfwSetCursorPosCallback  (p_window, cursorCallback);
    glfwSetScrollCallback     (p_window, scrollCallback);

    // Objets (après GLAD)
    Cube             cube;
    Grid             grid;
    GizmoTranslation gizmoTranslation;
    GizmoRotation    gizmoRotation;
    GizmoScale       gizmoScaleHandle;
    g_cube = &cube;

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(p_window))
    {
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glStencilMask(0xFF);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        const float aspect = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
        g_view = camera.getView();
        g_proj = camera.getProjection(aspect);

        gizmoVisualScale = glm::length(camera.getPosition() - cube.transform.position) * 0.18f;

        const DrawContext ctx { g_view, g_proj, camera.getPosition(), light };
        const glm::mat4   gridMVP = g_proj * g_view * glm::mat4(1.0f);

        grid.draw(gridMVP);
        cube.draw(ctx);
        if (cube.selected) {
            switch (activeGizmo) {
                case GizmoMode::Translation:
                    gizmoTranslation.draw(g_view, g_proj, cube.transform.position, gizmoVisualScale);
                    break;
                case GizmoMode::Rotation:
                    gizmoRotation.draw(g_view, g_proj, cube.transform.position, gizmoVisualScale);
                    break;
                case GizmoMode::Scale:
                    gizmoScaleHandle.draw(g_view, g_proj, cube.transform.position, gizmoVisualScale);
                    break;
            }
        }

        glfwSwapBuffers(p_window);
        glfwPollEvents();
    }

    glfwDestroyWindow(p_window);
    glfwTerminate();
    return EXIT_SUCCESS;
}

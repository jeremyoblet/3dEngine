#include "ViewerWidget3D.h"

#include <algorithm>
#include <limits>
#include <QTimer>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QOpenGLContext>
#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <glm/gtc/matrix_transform.hpp>

#include "Core/DrawContext.h"
#include "Objects/Lights/PointLightObject.h"
#include "Objects/Primitives/Cube.h"
#include "Objects/Display/Grid.h"
#include "Objects/Display/Gizmo/GizmoTranslation.h"
#include "Objects/Display/Gizmo/GizmoRotation.h"
#include "Objects/Display/Gizmo/GizmoScale.h"

// ---- Ctor / Dtor -----------------------------------------------------------

ViewerWidget3D::ViewerWidget3D(Scene& scene, QWidget* parent)
    : QOpenGLWidget(parent), m_scene(scene)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

ViewerWidget3D::~ViewerWidget3D() = default;

// ---- OpenGL lifecycle ------------------------------------------------------

void ViewerWidget3D::initializeGL()
{
    gladLoadGLLoader([](const char* name) -> void* {
        return reinterpret_cast<void*>(QOpenGLContext::currentContext()->getProcAddress(name));
    });

    m_grid             = std::make_unique<Grid>();
    m_gizmoTranslation = std::make_unique<GizmoTranslation>();
    m_gizmoRotation    = std::make_unique<GizmoRotation>();
    m_gizmoScale       = std::make_unique<GizmoScale>();

    glEnable(GL_DEPTH_TEST);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, QOverload<>::of(&ViewerWidget3D::update));
    m_timer->start(16);
}

void ViewerWidget3D::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void ViewerWidget3D::paintGL()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glStencilMask(0xFF);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    const float aspect = width() > 0 ? static_cast<float>(width()) / height() : 1.0f;
    m_view = m_camera.getView();
    m_proj = m_camera.getProjection(aspect);

    std::vector<PointLight> lights;
    traverseNodes(m_scene.roots(), [&](SceneNode& node) {
        if (auto* pl = dynamic_cast<PointLightObject*>(node.object.get()))
            lights.push_back({ node.object->transform.position, pl->lightColor, pl->intensity });
    });
    if (lights.empty())
        lights.push_back(m_light);

    const DrawContext ctx { m_view, m_proj, m_camera.getPosition(), lights };
    m_grid->draw(m_proj * m_view * glm::mat4(1.0f));
    m_scene.draw(ctx);

    if (m_selectedNode && m_selectedNode->object) {
        const glm::vec3& pos = m_selectedNode->object->transform.position;
        m_gizmoVisualScale = glm::length(m_camera.getPosition() - pos) * 0.18f;

        switch (m_activeGizmo) {
            case GizmoMode::Translation:
                m_gizmoTranslation->draw(m_view, m_proj, pos, m_gizmoVisualScale);
                break;
            case GizmoMode::Rotation:
                m_gizmoRotation->draw(m_view, m_proj, pos, m_gizmoVisualScale);
                break;
            case GizmoMode::Scale:
                m_gizmoScale->draw(m_view, m_proj, pos, m_gizmoVisualScale);
                break;
        }
    }
}

// ---- Selection -------------------------------------------------------------

void ViewerWidget3D::setSelection(std::vector<SceneNode*> nodes)
{
    for (auto* n : m_selection)
        if (n && n->object) n->object->selected = false;

    m_selection    = std::move(nodes);
    m_selectedNode = m_selection.empty() ? nullptr : m_selection.front();

    for (auto* n : m_selection)
        if (n && n->object) n->object->selected = true;

    emit selectionChanged(m_selectedNode);
}

void ViewerWidget3D::setSelectedNode(SceneNode* node)
{
    setSelection(node ? std::vector<SceneNode*>{ node } : std::vector<SceneNode*>{});
}

void ViewerWidget3D::selectNode(SceneNode* node)
{
    // Appelé depuis l'Outliner — on ne ré-émet pas selectionChanged pour éviter le cycle
    for (auto* n : m_selection)
        if (n && n->object) n->object->selected = false;

    m_selection    = node ? std::vector<SceneNode*>{ node } : std::vector<SceneNode*>{};
    m_selectedNode = node;

    for (auto* n : m_selection)
        if (n && n->object) n->object->selected = true;

    update();
}

void ViewerWidget3D::applyMarqueeSelection(const QRect& rect)
{
    std::vector<SceneNode*> selected;
    traverseNodes(m_scene.roots(), [&](SceneNode& node) {
        if (!node.object) return;

        const glm::vec3& pos = node.object->transform.position;

        // Rejeter les objets derrière la caméra
        glm::vec4 clip = m_proj * m_view * glm::vec4(pos, 1.0f);
        if (clip.w <= 0.0f) return;

        const glm::vec3& s    = node.object->transform.scale;
        float worldRadius     = 0.866f * std::max({ s.x, s.y, s.z });
        glm::vec2 cs          = worldToScreen(pos);

        // Rayon en pixels : max des 3 projections axis-aligned
        float screenRadius = 0.0f;
        for (const auto& axis : { glm::vec3(worldRadius,0,0), glm::vec3(0,worldRadius,0), glm::vec3(0,0,worldRadius) })
            screenRadius = std::max(screenRadius, glm::length(worldToScreen(pos + axis) - cs));

        if (cs.x - screenRadius >= rect.left()   &&
            cs.x + screenRadius <= rect.right()  &&
            cs.y - screenRadius >= rect.top()    &&
            cs.y + screenRadius <= rect.bottom())
        {
            selected.push_back(&node);
        }
    });

    setSelection(std::move(selected));
}

// ---- Public slot -----------------------------------------------------------

void ViewerWidget3D::addToScene(const std::string& name,
                                  std::function<std::unique_ptr<Object>()> factory)
{
    makeCurrent();
    m_scene.addObject(name, factory());
    doneCurrent();
    update();
    emit sceneChanged();
}

// ---- Scene traversal -------------------------------------------------------

void ViewerWidget3D::traverseNodes(const std::vector<std::unique_ptr<SceneNode>>& nodes,
                                    const std::function<void(SceneNode&)>& fn) const
{
    for (const auto& node : nodes) {
        fn(*node);
        traverseNodes(node->children, fn);
    }
}

SceneNode* ViewerWidget3D::pickNode(double mx, double my) const
{
    const glm::vec3 ro     = m_camera.getPosition();
    const glm::vec3 rd     = getCameraRay(mx, my);
    SceneNode*      result = nullptr;
    float           minDist = std::numeric_limits<float>::max();

    traverseNodes(m_scene.roots(), [&](SceneNode& node) {
        if (!node.object) return;
        const glm::vec3& s      = node.object->transform.scale;
        float            radius = 0.866f * std::max({ s.x, s.y, s.z });
        if (rayHitsSphere(ro, rd, node.object->transform.position, radius)) {
            float dist = glm::length(node.object->transform.position - ro);
            if (dist < minDist) { minDist = dist; result = &node; }
        }
    });

    return result;
}

// ---- Helpers ---------------------------------------------------------------

glm::vec3 ViewerWidget3D::getCameraRay(double mx, double my) const
{
    const float nx = 2.0f * (float)mx / width()  - 1.0f;
    const float ny = 1.0f - 2.0f * (float)my / height();
    glm::mat4 invPV = glm::inverse(m_proj * m_view);
    glm::vec4 near4 = invPV * glm::vec4(nx, ny, -1.0f, 1.0f); near4 /= near4.w;
    glm::vec4 far4  = invPV * glm::vec4(nx, ny,  1.0f, 1.0f); far4  /= far4.w;
    return glm::normalize(glm::vec3(far4) - glm::vec3(near4));
}

bool ViewerWidget3D::rayHitsSphere(glm::vec3 ro, glm::vec3 rd, glm::vec3 center, float radius) const
{
    glm::vec3 oc = ro - center;
    float b = glm::dot(oc, rd);
    float c = glm::dot(oc, oc) - radius * radius;
    return (b * b - c) >= 0.0f;
}

glm::vec2 ViewerWidget3D::worldToScreen(glm::vec3 p) const
{
    glm::vec4 clip = m_proj * m_view * glm::vec4(p, 1.0f);
    glm::vec3 ndc  = glm::vec3(clip) / clip.w;
    return { (ndc.x + 1.0f) * 0.5f * width(),
             (1.0f - ndc.y) * 0.5f * height() };
}

glm::vec2 ViewerWidget3D::screenAxisDir(int axis) const
{
    if (!m_selectedNode) return glm::vec2(0.0f);
    static constexpr glm::vec3 dirs[3] = { {1,0,0}, {0,1,0}, {0,0,1} };
    const glm::vec3& pos = m_selectedNode->object->transform.position;
    return worldToScreen(pos + dirs[axis]) - worldToScreen(pos);
}

int ViewerWidget3D::gizmoHitTest(double mx, double my) const
{
    if (!m_selectedNode) return -1;
    static constexpr glm::vec3 dirs[3] = { {1,0,0}, {0,1,0}, {0,0,1} };
    const glm::vec3& pos    = m_selectedNode->object->transform.position;
    const glm::vec2  mouse  = { (float)mx, (float)my };
    const glm::vec2  origin = worldToScreen(pos);
    constexpr float  THRESH = 12.0f;

    int   best = -1;
    float minD = THRESH;
    for (int i = 0; i < 3; ++i) {
        glm::vec2 tip     = worldToScreen(pos + dirs[i] * m_gizmoVisualScale);
        glm::vec2 seg     = tip - origin;
        float     segLen2 = glm::dot(seg, seg);
        if (segLen2 < 1.0f) continue;
        float t = glm::clamp(glm::dot(mouse - origin, seg) / segLen2, 0.0f, 1.0f);
        float d = glm::length(mouse - (origin + t * seg));
        if (d < minD) { minD = d; best = i; }
    }
    return best;
}

int ViewerWidget3D::scaleHitTest(double mx, double my) const
{
    return gizmoHitTest(mx, my);
}

int ViewerWidget3D::rotationHitTest(double mx, double my) const
{
    if (!m_selectedNode) return -1;
    constexpr int   N      = 64;
    constexpr float TWO_PI = 6.28318530718f;

    struct Ring { glm::vec3 u, v; };
    constexpr Ring rings[3] = {
        { {0,1,0}, {0,0,1} },
        { {1,0,0}, {0,0,1} },
        { {1,0,0}, {0,1,0} },
    };

    const glm::vec2  mouse  = { (float)mx, (float)my };
    const glm::vec3& pos    = m_selectedNode->object->transform.position;
    constexpr float  THRESH = 10.0f;

    int   best = -1;
    float minD = THRESH;
    for (int ri = 0; ri < 3; ++ri) {
        for (int i = 0; i < N; ++i) {
            float a0 = TWO_PI * i       / N;
            float a1 = TWO_PI * (i + 1) / N;
            glm::vec3 p0 = pos + (std::cos(a0) * rings[ri].u + std::sin(a0) * rings[ri].v) * m_gizmoVisualScale;
            glm::vec3 p1 = pos + (std::cos(a1) * rings[ri].u + std::sin(a1) * rings[ri].v) * m_gizmoVisualScale;
            glm::vec2 s0 = worldToScreen(p0);
            glm::vec2 s1 = worldToScreen(p1);
            glm::vec2 seg     = s1 - s0;
            float     segLen2 = glm::dot(seg, seg);
            if (segLen2 < 0.1f) continue;
            float t = glm::clamp(glm::dot(mouse - s0, seg) / segLen2, 0.0f, 1.0f);
            float d = glm::length(mouse - (s0 + t * seg));
            if (d < minD) { minD = d; best = ri; }
        }
    }
    return best;
}

// ---- Events ----------------------------------------------------------------

void ViewerWidget3D::paintEvent(QPaintEvent* event)
{
    QOpenGLWidget::paintEvent(event); // composite le rendu OpenGL

    if (m_marqueeActive) {
        QRect rect = QRect(m_marqueeStart, QPoint((int)m_lastX, (int)m_lastY)).normalized();
        QPainter painter(this);
        painter.setPen(QPen(QColor(100, 170, 255, 200), 1));
        painter.setBrush(QColor(100, 170, 255, 30));
        painter.drawRect(rect);
    }
}

void ViewerWidget3D::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
        case Qt::Key_W: m_activeGizmo = GizmoMode::Translation; break;
        case Qt::Key_E: m_activeGizmo = GizmoMode::Rotation;    break;
        case Qt::Key_R: m_activeGizmo = GizmoMode::Scale;       break;
        case Qt::Key_Z:
            if (m_selectedNode && m_selectedNode->object)
                m_selectedNode->object->wireframe = !m_selectedNode->object->wireframe;
            break;
        case Qt::Key_F:
            if (m_selectedNode && m_selectedNode->object) {
                const glm::vec3& s = m_selectedNode->object->transform.scale;
                m_camera.focusOn(m_selectedNode->object->transform.position,
                                 std::max({ s.x, s.y, s.z }));
            }
            break;
        default:
            QOpenGLWidget::keyPressEvent(event);
    }
}

void ViewerWidget3D::mousePressEvent(QMouseEvent* event)
{
    const double mx      = event->position().x();
    const double my      = event->position().y();
    const bool   altHeld = event->modifiers() & Qt::AltModifier;

    if (event->button() == Qt::LeftButton) {
        if (altHeld) {
            m_leftDown = true; // Alt + gauche → orbite
        } else {
            bool gizmoConsumed = false;
            if (m_selectedNode) {
                switch (m_activeGizmo) {
                    case GizmoMode::Translation:
                        m_dragAxis = gizmoHitTest(mx, my);
                        gizmoConsumed = m_dragAxis >= 0;
                        break;
                    case GizmoMode::Rotation:
                        m_rotationDragAxis = rotationHitTest(mx, my);
                        gizmoConsumed = m_rotationDragAxis >= 0;
                        break;
                    case GizmoMode::Scale:
                        m_scaleDragAxis = scaleHitTest(mx, my);
                        gizmoConsumed = m_scaleDragAxis >= 0;
                        break;
                }
            }
            if (!gizmoConsumed) {
                SceneNode* hit = pickNode(mx, my);
                if (hit) {
                    setSelectedNode(hit);
                } else {
                    m_marqueeActive = true;
                    m_marqueeStart  = QPoint((int)mx, (int)my);
                }
            }
        }
    }

    if (event->button() == Qt::MiddleButton && altHeld)
        m_middleDown = true; // Alt + milieu → pan

    m_lastX = mx;
    m_lastY = my;
}

void ViewerWidget3D::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_marqueeActive) {
            QRect rect = QRect(m_marqueeStart, QPoint((int)event->position().x(),
                                                      (int)event->position().y())).normalized();
            if (rect.width() > 4 || rect.height() > 4)
                applyMarqueeSelection(rect);
            else
                setSelectedNode(nullptr); // clic dans le vide → désélectionner
            m_marqueeActive = false;
        }
        m_leftDown         = false;
        m_dragAxis         = -1;
        m_rotationDragAxis = -1;
        m_scaleDragAxis    = -1;
    }
    if (event->button() == Qt::MiddleButton)
        m_middleDown = false;
}

void ViewerWidget3D::mouseMoveEvent(QMouseEvent* event)
{
    const double x  = event->position().x();
    const double y  = event->position().y();
    const float  dx = static_cast<float>(x - m_lastX);
    const float  dy = static_cast<float>(y - m_lastY);

    static constexpr glm::vec3 axisDirs[3] = { {1,0,0}, {0,1,0}, {0,0,1} };

    if (m_selectedNode && m_dragAxis >= 0) {
        glm::vec2 sa  = screenAxisDir(m_dragAxis);
        float     len = glm::length(sa);
        if (len > 1.0f) {
            float delta = (dx * sa.x + dy * sa.y) / (len * len);
            for (auto* n : m_selection)
                if (n && n->object)
                    n->object->transform.position += axisDirs[m_dragAxis] * delta;
        }
    } else if (m_selectedNode && m_rotationDragAxis >= 0) {
        const glm::vec3& pos = m_selectedNode->object->transform.position;
        glm::vec2 sa  = worldToScreen(pos + axisDirs[m_rotationDragAxis]) - worldToScreen(pos);
        float     len = glm::length(sa);
        if (len > 1.0f) {
            glm::vec2 perp  = glm::vec2(-sa.y, sa.x) / len;
            float     delta = (dx * perp.x + dy * perp.y) * 0.5f;
            for (auto* n : m_selection)
                if (n && n->object)
                    n->object->transform.rotation[m_rotationDragAxis] += delta;
        }
    } else if (m_selectedNode && m_scaleDragAxis >= 0) {
        glm::vec2 sa  = screenAxisDir(m_scaleDragAxis);
        float     len = glm::length(sa);
        if (len > 1.0f) {
            float delta = (dx * sa.x + dy * sa.y) / (len * len);
            for (auto* n : m_selection)
                if (n && n->object) {
                    float& s = n->object->transform.scale[m_scaleDragAxis];
                    s = std::max(0.01f, s + delta * 1.5f);
                }
        }
    } else {
        if (m_leftDown)     m_camera.orbit(dx, dy);
        if (m_middleDown)   m_camera.pan(dx, dy);
        if (m_marqueeActive) update(); // redessine le rectangle
    }

    m_lastX = x;
    m_lastY = y;
}

void ViewerWidget3D::wheelEvent(QWheelEvent* event)
{
    m_camera.zoom(event->angleDelta().y() / 120.0f);
}

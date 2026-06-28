#pragma once
#include <glad/glad.h>  // doit précéder tout header Qt/OpenGL
#include <memory>
#include <functional>
#include <QOpenGLWidget>
#include <glm/glm.hpp>
#include "Core/Scene.h"
#include "Objects/Cameras/Camera.h"
#include "Objects/Lights/PointLight.h"

class Grid;
class GizmoTranslation;
class GizmoRotation;
class GizmoScale;
class QTimer;

class ViewerWidget3D : public QOpenGLWidget {
    Q_OBJECT

public:
    explicit ViewerWidget3D(Scene& scene, QWidget* parent = nullptr);
    ~ViewerWidget3D() override;

    void addToScene(const std::string& name,
                    std::function<std::unique_ptr<Object>()> factory);
    void selectNode(SceneNode* node);

signals:
    void sceneChanged();
    void selectionChanged(SceneNode* node);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    Scene&     m_scene;
    SceneNode* m_selectedNode = nullptr;

    Camera     m_camera;
    PointLight m_light;

    std::unique_ptr<Grid>             m_grid;
    std::unique_ptr<GizmoTranslation> m_gizmoTranslation;
    std::unique_ptr<GizmoRotation>    m_gizmoRotation;
    std::unique_ptr<GizmoScale>       m_gizmoScale;

    float     m_gizmoVisualScale  = 1.0f;
    glm::mat4 m_view              { 1.0f };
    glm::mat4 m_proj              { 1.0f };

    bool   m_leftDown         = false;
    bool   m_middleDown       = false;
    double m_lastX            = 0.0;
    double m_lastY            = 0.0;
    int    m_dragAxis         = -1;
    int    m_rotationDragAxis = -1;
    int    m_scaleDragAxis    = -1;

    enum class GizmoMode { Translation, Rotation, Scale };
    GizmoMode m_activeGizmo = GizmoMode::Translation;

    QTimer* m_timer = nullptr;

    void       setSelectedNode(SceneNode* node); // modifie la sélection et émet selectionChanged

    SceneNode* pickNode(double mx, double my) const;
    void       traverseNodes(const std::vector<std::unique_ptr<SceneNode>>& nodes,
                             const std::function<void(SceneNode&)>& fn) const;

    glm::vec3 getCameraRay(double mx, double my) const;
    bool      rayHitsSphere(glm::vec3 ro, glm::vec3 rd, glm::vec3 center, float radius) const;
    glm::vec2 worldToScreen(glm::vec3 p) const;
    glm::vec2 screenAxisDir(int axis) const;
    int       gizmoHitTest(double mx, double my) const;
    int       scaleHitTest(double mx, double my) const;
    int       rotationHitTest(double mx, double my) const;
};

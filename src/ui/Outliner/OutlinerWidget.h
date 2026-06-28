#pragma once
#include <vector>
#include <QWidget>
#include "Core/Scene.h"

class QTreeView;
class SceneModel;

class OutlinerWidget : public QWidget {
    Q_OBJECT

public:
    explicit OutlinerWidget(const Scene& scene, QWidget* parent = nullptr);

    void refresh();
    void selectNodes(const std::vector<SceneNode*>& nodes);

signals:
    void selectionChanged(const std::vector<SceneNode*>& nodes);

private:
    SceneModel* m_model;
    QTreeView*  m_view;
    bool        m_blockSelectionSignal = false;
};

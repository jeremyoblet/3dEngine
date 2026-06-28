#pragma once
#include <QWidget>
#include "Core/Scene.h"

class QTreeView;
class SceneModel;

class OutlinerWidget : public QWidget {
    Q_OBJECT

public:
    explicit OutlinerWidget(const Scene& scene, QWidget* parent = nullptr);

private:
    SceneModel* m_model;
    QTreeView*  m_view;
};

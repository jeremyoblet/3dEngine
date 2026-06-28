#include "OutlinerWidget.h"
#include "SceneModel.h"
#include <QTreeView>
#include <QVBoxLayout>

OutlinerWidget::OutlinerWidget(const Scene& scene, QWidget* parent)
    : QWidget(parent)
    , m_model(new SceneModel(scene, this))
    , m_view(new QTreeView(this))
{
    m_view->setModel(m_model);
    m_view->setHeaderHidden(true);
    m_view->setExpandsOnDoubleClick(true);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);
}

void OutlinerWidget::refresh()
{
    m_model->refresh();
}

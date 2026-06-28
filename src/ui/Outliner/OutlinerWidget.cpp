#include "OutlinerWidget.h"
#include "SceneModel.h"
#include <QTreeView>
#include <QVBoxLayout>
#include <QItemSelectionModel>

OutlinerWidget::OutlinerWidget(const Scene& scene, QWidget* parent)
    : QWidget(parent)
    , m_model(new SceneModel(scene, this))
    , m_view(new QTreeView(this))
{
    m_view->setModel(m_model);
    m_view->setHeaderHidden(true);
    m_view->setExpandsOnDoubleClick(true);

    connect(m_view->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this](const QItemSelection& selected, const QItemSelection&) {
        const auto indexes = selected.indexes();
        SceneNode* node = indexes.isEmpty()
            ? nullptr
            : static_cast<SceneNode*>(indexes.first().internalPointer());
        emit selectionChanged(node);
    });

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);
}

void OutlinerWidget::refresh()
{
    m_model->refresh();
}

void OutlinerWidget::selectNode(SceneNode* node)
{
    QSignalBlocker blocker(m_view->selectionModel());
    if (!node) {
        m_view->clearSelection();
        return;
    }
    QModelIndex idx = m_model->indexForNode(node);
    if (idx.isValid()) {
        m_view->setCurrentIndex(idx);
        m_view->scrollTo(idx);
    }
}

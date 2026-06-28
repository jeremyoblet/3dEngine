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
    m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(m_view->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this](const QItemSelection&, const QItemSelection&) {
        if (m_blockSelectionSignal) return;
        const auto indexes = m_view->selectionModel()->selectedIndexes();
        std::vector<SceneNode*> nodes;
        nodes.reserve(indexes.size());
        for (const auto& idx : indexes)
            if (idx.isValid())
                nodes.push_back(static_cast<SceneNode*>(idx.internalPointer()));
        emit selectionChanged(nodes);
    });

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);
}

void OutlinerWidget::refresh()
{
    m_model->refresh();
}

void OutlinerWidget::selectNodes(const std::vector<SceneNode*>& nodes)
{
    m_blockSelectionSignal = true;
    m_view->clearSelection();
    for (auto* node : nodes) {
        QModelIndex idx = m_model->indexForNode(node);
        if (idx.isValid())
            m_view->selectionModel()->select(idx, QItemSelectionModel::Select);
    }
    if (!nodes.empty()) {
        QModelIndex primary = m_model->indexForNode(nodes.front());
        if (primary.isValid()) m_view->scrollTo(primary);
    }
    m_blockSelectionSignal = false;
}

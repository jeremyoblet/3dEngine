#pragma once
#include <QAbstractItemModel>
#include "Core/Scene.h"

class SceneModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit SceneModel(const Scene& scene, QObject* parent = nullptr);

    QModelIndex index(int row, int col, const QModelIndex& parent = {}) const override;
    QModelIndex parent(const QModelIndex& index)                          const override;
    int         rowCount(const QModelIndex& parent = {})                  const override;
    int         columnCount(const QModelIndex& parent = {})               const override;
    QVariant    data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void refresh();

private:
    const Scene& m_scene;
};

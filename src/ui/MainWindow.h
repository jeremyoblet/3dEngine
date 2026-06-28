#pragma once
#include <QMainWindow>
#include "MenuContext.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

    void applyMenuContext(const MenuContext& ctx);
};

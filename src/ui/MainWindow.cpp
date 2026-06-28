#include "MainWindow.h"
#include <QMenuBar>
#include <QAction>
#include <QMenu>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("3D Engine");
    resize(1280, 720);
}

void MainWindow::applyMenuContext(const MenuContext& ctx)
{
    menuBar()->clear();

    for (const auto& entry : ctx.menus()) {
        QMenu* menu = menuBar()->addMenu(entry.title);
        for (const auto& action : entry.actions) {
            if (action.separator) {
                menu->addSeparator();
            } else {
                QAction* qa = menu->addAction(action.label);
                connect(qa, &QAction::triggered, [fn = action.trigger] { fn(); });
            }
        }
    }
}

#include <QApplication>
#include <QSurfaceFormat>
#include <QSplitter>

#include "ui/MainWindow.h"
#include "ui/MenuContext.h"
#include "ui/Outliner/OutlinerWidget.h"
#include "ui/Viewers/3d/ViewerWidget3D.h"
#include "Core/Scene.h"

int main(int argc, char* argv[])
{
    QSurfaceFormat format;
    format.setVersion(4, 5);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);

    Scene scene;

    MenuContext menuCtx;
    menuCtx.addMenu("File", {
        { "New Scene", [] { /* TODO */ } },
        MenuAction::Separator(),
        { "Quit",      [] { QApplication::quit(); } },
    });
    menuCtx.addMenu("Edit", {
        { "Undo", [] { /* TODO */ } },
        { "Redo", [] { /* TODO */ } },
    });
    menuCtx.addMenu("View", {
        { "Outliner", [] { /* TODO */ } },
    });

    MainWindow window;
    window.applyMenuContext(menuCtx);

    auto* splitter = new QSplitter(Qt::Horizontal, &window);
    splitter->addWidget(new OutlinerWidget(scene, &window));
    splitter->addWidget(new ViewerWidget3D(&window));
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 4);

    window.setCentralWidget(splitter);
    window.show();

    return app.exec();
}

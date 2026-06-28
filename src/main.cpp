#include <QApplication>
#include <QSurfaceFormat>
#include <QSplitter>

#include "ui/MainWindow.h"
#include "ui/MenuContext.h"
#include "ui/Outliner/OutlinerWidget.h"
#include "ui/Viewers/3d/ViewerWidget3D.h"
#include "Core/Scene.h"
#include "Objects/Primitives/Cube.h"
#include "Objects/Primitives/Triangle.h"
#include "Objects/Primitives/Sphere.h"
#include "Objects/Lights/PointLightObject.h"

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

    auto* viewer   = new ViewerWidget3D(scene);
    auto* outliner = new OutlinerWidget(scene);

    MenuContext menuCtx;
    menuCtx.addMenu("File", {
        { "New Scene", [] { /* TODO */ } },
        MenuAction::Separator(),
        { "Quit", [] { QApplication::quit(); } },
    });
    menuCtx.addMenu("Edit", {
        { "Undo", [] { /* TODO */ } },
        { "Redo", [] { /* TODO */ } },
    });
    menuCtx.addMenu("View", {
        { "Outliner", [] { /* TODO */ } },
    });
    menuCtx.addMenu("Création", {
        { "Cube",     [viewer] { viewer->addToScene("Cube",     [] { return std::make_unique<Cube>(); }); }},
        { "Sphère",   [viewer] { viewer->addToScene("Sphère",   [] { return std::make_unique<Sphere>(); }); }},
        { "Triangle",     [viewer] { viewer->addToScene("Triangle",     [] { return std::make_unique<Triangle>(); }); }},
        MenuAction::Separator(),
        { "Point Light",  [viewer] { viewer->addToScene("Point Light", [] { return std::make_unique<PointLightObject>(); }); }},
    });

    QObject::connect(viewer,   &ViewerWidget3D::sceneChanged,
                     outliner, &OutlinerWidget::refresh);
    QObject::connect(viewer,   &ViewerWidget3D::selectionChanged,
                     outliner, &OutlinerWidget::selectNode);
    QObject::connect(outliner, &OutlinerWidget::selectionChanged,
                     viewer,   &ViewerWidget3D::selectNode);

    MainWindow window;
    window.applyMenuContext(menuCtx);

    auto* splitter = new QSplitter(Qt::Horizontal, &window);
    splitter->addWidget(outliner);
    splitter->addWidget(viewer);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 4);

    window.setCentralWidget(splitter);
    window.show();

    return app.exec();
}

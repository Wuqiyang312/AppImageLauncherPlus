#include <QApplication>
#include "appimage_manager.h"
#include "../shared/shared.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    app.setApplicationName("AppImageLauncher Manager");
    app.setApplicationDisplayName("AppImage Manager");
    app.setApplicationVersion("1.0.0");

    AppImageManager manager;
    setUpFallbackIconPaths(&manager);
    manager.show();

    return app.exec();
}

#include <QApplication>
#include "MainWindow.h"
#include <QFile>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Apply basic dark theme application-wide
    app.setStyle("Fusion");

    MainWindow window;
    window.setWindowTitle("PyPixl");
    window.resize(1024, 768);
    window.show();

    return app.exec();
}

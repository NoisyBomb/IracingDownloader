#include <QApplication>
#include "ui/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("Grid-and-Go Setup Manager");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("GnG");

    MainWindow w;
    w.show();

    return app.exec();
}

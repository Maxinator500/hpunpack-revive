#include "HPU_Qt.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    HPU_Qt window;
    window.show();
    return app.exec();
}

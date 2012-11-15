#include <QtGui/QApplication>
#include "cam_interface_test.h"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_X11InitThreads);
    cam_interface_test foo;
    foo.show();
    return app.exec();
}

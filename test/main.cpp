#include <QtGui/QApplication>
#include "cam_interface_test.h"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    cam_interface_test foo;
    foo.show();
    return app.exec();
}

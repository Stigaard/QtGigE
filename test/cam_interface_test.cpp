#include "cam_interface_test.h"
#include "cam_interface_test.moc"

#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QAction>
#include <QtGui/QStatusBar>
#include <QString>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

cam_interface_test::cam_interface_test()
{
    getitimer(ITIMER_PROF, &ptimer);
    qRegisterMetaType< cv::Mat >("cv::Mat");    
    std::cout << "CAM_interface_test TID:" << syscall(SYS_gettid) << std::endl << std::flush;
    cam = new BASLER_ACA2000("21272795");
    cam->setptimer(ptimer);
    
    //this->cam->setROI(0,0,1000,1000);
    connect(this->cam, SIGNAL(newBayerGRImage(cv::Mat)), &dem, SLOT(newBayerGRImage(cv::Mat)), Qt::QueuedConnection);
    connect(&dem, SIGNAL(newImage(cv::Mat)), &view, SLOT(showImage(cv::Mat)));
    connect(this->cam, SIGNAL(measuredFPS(float)), this, SLOT(measured_cam_fps(float)));
    this->cam->startAquisition();
    this->setCentralWidget(&view);
}

cam_interface_test::~cam_interface_test()
{

}

void cam_interface_test::measured_cam_fps(float fps)
{
  this->statusBar()->showMessage(QString("Camera:" + QString::number(fps) + "Hz"));
}

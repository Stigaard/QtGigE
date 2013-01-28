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
    cam = new QTGIGE("21272795");
    cam->setptimer(ptimer);
    
    //this->cam->setROI(0,0,1000,1000);
    //connect(this->cam, SIGNAL(newBayerGRImage(cv::Mat)), &dem, SLOT(newBayerGRImage(cv::Mat)), Qt::QueuedConnection);
    //connect(&dem, SIGNAL(newImage(cv::Mat)), &view, SLOT(showImage(cv::Mat)));
    connect(this->cam, SIGNAL(measuredFPS(float)), this, SLOT(measured_cam_fps(float)));
    //connect(this->cam, SIGNAL(measuredFrameStats(int,int)), this, SLOT(measured_cam_frame_stats(int,int)));
    this->cam->startAquisition();
    this->setCentralWidget(&view);
    //this->cam->PrintParms();
}

cam_interface_test::~cam_interface_test()
{

}

void cam_interface_test::measured_cam_fps(float fps)
{
  this->statusBar()->showMessage(QString("Camera:" + QString::number(fps) + "Hz"));
}

void cam_interface_test::measured_cam_frame_stats(int success, int failed)
{
  this->statusBar()->showMessage(QString("Successfull frames:" + QString::number(success) + " Failed frames:" + QString::number(failed) + " == " + QString::number(100*((float)failed)/((float)(success+failed))) + "% failed"));
}


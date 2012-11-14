#include "cam_interface_test.h"
#include "cam_interface_test.moc"

#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QAction>

cam_interface_test::cam_interface_test()
{
    qRegisterMetaType< cv::Mat >("cv::Mat");
    
    cam = new BASLER_ACA2000("21272795");
    //this->cam->setROI(0,0,1000,1000);
 //   connect(this->cam, SIGNAL(newBayerBGImage(cv::Mat)), this, SLOT(newBayerBGImage(cv::Mat)));
    connect(this->cam, SIGNAL(newBayerGRImage(cv::Mat)), this, SLOT(newBayerGRImage(cv::Mat)), Qt::DirectConnection);
    connect(this, SIGNAL(newImage(cv::Mat)), &view, SLOT(showImage(cv::Mat)));
    this->cam->startAquisition();
    
    
    this->setCentralWidget(&view);
}

cam_interface_test::~cam_interface_test()
{}

void cam_interface_test::newBayerBGImage(cv::Mat img)
{
  cv::Mat rgb;
  cv::cvtColor(img, rgb, CV_BayerBG2BGR);
  emit(newImage(rgb));
}


void cam_interface_test::newBayerGRImage(cv::Mat img)
{
  cv::Mat rgb;
  cv::Mat BayerGR8(img.rows,img.cols, cv::DataType<uint8_t>::type);
  img.convertTo(BayerGR8, BayerGR8.type(), 1.0/256.0);
  cv::cvtColor(BayerGR8, rgb, CV_BayerGR2BGR);
  emit(newImage(rgb));
}
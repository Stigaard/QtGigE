#ifndef cam_interface_test_H
#define cam_interface_test_H

#include <QtGui/QMainWindow>
#include <opencv2/opencv.hpp>
#include "../basler_acA2000-50gc.h"
#include "cqtopencvviewergl.h"

class cam_interface_test : public QMainWindow
{
Q_OBJECT
public:
    cam_interface_test();
    virtual ~cam_interface_test();
private:
  BASLER_ACA2000 * cam;
  CQtOpenCVViewerGl view;
private slots:
  void newBayerBGImage(cv::Mat img);
  void newBayerGRImage(cv::Mat img);
signals:
  void newImage(cv::Mat img);
};

#endif // cam_interface_test_H

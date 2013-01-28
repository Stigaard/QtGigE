#ifndef cam_interface_test_H
#define cam_interface_test_H

#include <QtGui/QMainWindow>
#include <opencv2/opencv.hpp>
#include <sys/time.h>
#include "../basler_acA2000-50gc.h"
#include "cqtopencvviewergl.h"
#include "demosaic_cv.h"

class cam_interface_test : public QMainWindow
{
Q_OBJECT
public:
    cam_interface_test();
    ~cam_interface_test();
private:
  QTGIGE * cam;
  CQtOpenCVViewerGl view;
  demosaic_cv dem;
  itimerval ptimer;
private slots:
  void measured_cam_fps(float fps);
  void measured_cam_frame_stats(int success, int failed);
};

#endif // cam_interface_test_H

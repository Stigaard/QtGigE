// Copyright (c) 2011 University of Southern Denmark. All rights reserved.
// Use of this source code is governed by the MIT license (see license.txt).
#ifndef CAMERA_BASLER_ACA2000_50GC_
#define CAMERA_BASLER_ACA2000_50GC_
#include <arvtypes.h>
#include <arvstream.h>
#include <QObject>
#include <QMutex>
#include <QQueue>
#include <QSemaphore>

#include <time.h>
#include <QThread>
#include <opencv2/opencv.hpp>
  class BASLER_ACA2000 : public QThread {
    Q_OBJECT
    public:
    BASLER_ACA2000(char* serial);
    ~BASLER_ACA2000();
    int setROI(int x, int y, int width, int height);
    int setExposure(float period); //Exposure time in µs
    int setGain(float gain); //Unit currently unknown (have to look it up from datasheet)
    void newImageCallback(ArvStreamCallbackType type, ArvBuffer* buffer);
    static void newImageCallbackWrapper(void *user_data, ArvStreamCallbackType type, ArvBuffer *buffer);
    int startAquisition(void);
    int stopAquisition(void);
    void unpack12BitPacked(const ArvBuffer* img, char* unpacked16);
  signals:
    void newBayerBGImage(const cv::Mat img);
    void newBayerGRImage(const cv::Mat img);
  private:
      void run();
      ArvCamera * camera;
      ArvStream* stream;
      QQueue<ArvBuffer*> bufferQue;
      QSemaphore bufferSem;
  };

#endif  // CAMERA_BASLER_ACA2000_50GC_


// Copyright (c) 2011 University of Southern Denmark. All rights reserved.
// Use of this source code is governed by the MIT license (see license.txt).
#ifndef QTGIGE_H
#define QTGIGE_H
#ifdef EMULATE_CAMERA
  #define EMULATION_INPUT_FILE "../simulation_image/maizeAnd3weedsCombinedImage.png"
#else
  #include <arvtypes.h>
  #include <arvstream.h>
#endif
#include <QObject>
#include <QMutex>
#include <QQueue>
#include <QSemaphore>
#include <QElapsedTimer>
#include <QTreeWidgetItem>
#include <QGridLayout>
#include <QDialog>

#include <time.h>
#include <sys/time.h>
#include <QThread>
#include <qvarlengtharray.h>
#include <qsignalmapper.h>
#include <opencv2/opencv.hpp>
  class QTGIGE : public QThread {
    Q_OBJECT
    public:
    QTGIGE(char* deviceId);
    ~QTGIGE();
    int setROI(int x, int y, int width, int height);
    int setExposure(float period); //Exposure time in µs
    int setGain(float gain); //Unit currently unknown (have to look it up from datasheet)
#ifndef EMULATE_CAMERA
    void newImageCallback(ArvStreamCallbackType type, ArvBuffer* buffer);
    static void newImageCallbackWrapper(void *user_data, ArvStreamCallbackType type, ArvBuffer *buffer);
    void unpack12BitPacked(const ArvBuffer* img, char* unpacked16);
#endif
    int startAquisition(void);
    int stopAquisition(void);
    void setptimer(itimerval timer);
    void PrintParms(void);
    static void convert16to8bit(cv::InputArray in, cv::OutputArray out);
  signals:
    void newBayerGRImage(const cv::Mat img, qint64 timestampus);
    void measuredFPS(float fps);
    void measuredFrameStats(int success, int failed);
  public slots:
    void showCameraSettings(void);
    void writeEnum(QString nodeName, QString value);
    void writeFloat(QString nodeName, float value);
    void writeInt(QString nodeName, int value);
    void writeBool(QString nodeName, bool value);
    void emitAction(QString nodeName);
  private:
      void run();
#ifndef EMULATE_CAMERA
      ArvCamera * camera;
      ArvStream* stream;
      ArvDevice * dev;
      ArvGc *genicam;
      QQueue<ArvBuffer*> bufferQue;
      qint64 offset;
#endif
#ifdef EMULATE_CAMERA
      int roi_cpos;
#endif
      int roi_width;
      int roi_height;
      int roi_x;
      int roi_y;
      double roi_scale;
      QSemaphore bufferSem;
      itimerval ptimer;
      bool updateptimer;
      bool abort;
      static const int frameAvg = 20;
      QElapsedTimer framePeriod;
      int nFrames;
      int successFrames;
      int failedFrames;
#ifndef EMULATE_CAMERA
      void gigE_list_features(ArvGc* genicam, const char* feature, gboolean show_description, QTreeWidgetItem* parent);
#endif
      QDialog *settings;
      QWidget * currentSetting;
      QTreeWidget *treeWidget;
      QGridLayout *settingsLayout;
      QGridLayout * currentSettingLayout;
      void drawSettingsDialog(void);
  private slots:
      void newSettingSelected(QTreeWidgetItem* item,int column);
      void writeEnumFromSettingsSelectorMapper(QString value);
      void writeFloatFromSettings(int value);
      void writeIntFromSettings(int value);
      void writeBoolFromSettings(int value);
      void emitActionFromSettings(void);
  };

#endif  // QTGIGE_H


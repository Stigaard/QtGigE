/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef DEMOSAIC_CV_H
#define DEMOSAIC_CV_H

#include <qt4/QtCore/QThread>
#include <qt4/QtCore/QSemaphore>
#include <qt4/QtCore/QQueue>
#include <opencv2/opencv.hpp>

#include <sys/time.h>

class demosaic_cv : public QThread
{
  Q_OBJECT
public:
  demosaic_cv(void);
  ~demosaic_cv();
  void run();
  void setptimer(itimerval timer);
public slots:
  void newBayerGRImage(cv::Mat img);
signals:
  void newImage(cv::Mat img);
private:
  QSemaphore semImg;
  QQueue<cv::Mat> Imgs;
  itimerval ptimer;
  bool updateptimer;
  bool abort;
};

#endif // DEMOSAIC_CV_H

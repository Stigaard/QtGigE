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


#include "demosaic_cv.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "demosaic_cv.moc"
demosaic_cv::demosaic_cv(void )
{
  updateptimer = false;
  abort = false;
  this->start();
}

demosaic_cv::~demosaic_cv()
{
  abort = true;
  this->msleep(300);
}


void demosaic_cv::setptimer(itimerval timer)
{
  setitimer(ITIMER_PROF, &timer, NULL);
  ptimer = timer;
  updateptimer = true;
}


void demosaic_cv::run()
{
  this->msleep(400);
  std::cout << "DEMOSAIC TID:" << syscall(SYS_gettid) << std::endl << std::flush;
  while(abort==false)
  {
    if(updateptimer)
    {
      setitimer(ITIMER_PROF, &ptimer, NULL);
      updateptimer = false;
    }
    bool cont = false;
    while(cont == false)
    {
      cont = semImg.tryAcquire(1,100);
      if(abort == true)
	return;
    }
    cv::Mat img = Imgs.dequeue();
    cv::Mat rgb;
    cv::Mat BayerGR8(img.rows,img.cols, cv::DataType<uint8_t>::type);
    img.convertTo(BayerGR8, BayerGR8.type(), 1.0/256.0);
    cv::cvtColor(BayerGR8, rgb, CV_BayerGR2BGR);
    emit(newImage(rgb));    
  }
}

void demosaic_cv::newBayerGRImage(cv::Mat img)
{
  Imgs.enqueue(img);
  semImg.release(1);
}

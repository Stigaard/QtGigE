// Copyright (c) 2011 University of Southern Denmark. All rights reserved.
// Use of this source code is governed by the MIT license (see license.txt).
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <semaphore.h>
#include <QElapsedTimer>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "basler_acA2000-50gc.moc"
#include "basler_acA2000-50gc.h"
#undef signals
  #include <arv.h>

BASLER_ACA2000::BASLER_ACA2000(char* serial)
{
  this->camera = arv_camera_new (NULL);
  updateptimer = false;
//   std::cout << "Vendor name:" << arv_camera_get_vendor_name (camera) << std::endl;
//   std::cout << "Model name:" << arv_camera_get_model_name (camera) << std::endl;
//   std::cout << "Device ID:" << arv_camera_get_device_id (camera) << std::endl;
  abort = false;
  this->start();
}

void BASLER_ACA2000::setptimer(itimerval timer)
{
  setitimer(ITIMER_PROF, &timer, NULL	);
  ptimer = timer;
  updateptimer = true;
}


BASLER_ACA2000::~BASLER_ACA2000()
{
  abort = true;
  this->msleep(300);
  g_object_unref(this->camera);
}


int BASLER_ACA2000::setROI(int x, int y, int width, int height)
{
  arv_camera_set_region (camera, x, y, width, height);
}

int BASLER_ACA2000::setExposure(float period)
{
  arv_camera_set_exposure_time (camera, period);
}

int BASLER_ACA2000::setGain(float gain)
{
  arv_camera_set_gain (camera, gain);
}

void BASLER_ACA2000::unpack12BitPacked(const ArvBuffer* img, char* unpacked16)
{
  unsigned char * img_ = (unsigned char*)(img->data);
  unsigned char * end = (unsigned char*)img->data + (img->width*img->height*3)/2;
  unsigned char b0,b1,b2;
  uint16_t *out = (uint16_t *)unpacked16;
  while(img_!=end)
  {
    b0 = *img_++;
    b1 = *img_++;
    b2 = *img_++;
    *out++ = ((b1 && 0x0f)<<4) + (b0<<8);
    *out++ = (b1 && 0xf0) + (b2<<8);
  }
}

void BASLER_ACA2000::convert16to8bit(cv::InputArray in, cv::OutputArray out)
{
  uint16_t * in_ = (uint16_t*)in.getMat().ptr();
  cv::Mat tmp_out(in.getMat().size().height, in.getMat().size().width, cv::DataType<uint8_t>::type);
  uint8_t * out_ = tmp_out.ptr();
  uint16_t * end = in_ + (in.getMat().size().height * in.getMat().size().width);
  while(in_!=end)
  {
    *out_++ = (*in_++)>>8;
  }
  tmp_out.copyTo(out);
}

void BASLER_ACA2000::newImageCallbackWrapper(void* user_data, ArvStreamCallbackType type, ArvBuffer* buffer)
{
  //Please note that this is in fact a static method, where we pass the instance as a user_data parameter,
  //therefore all ops should be performed on the This object, instead of the this (which shouldn't exist)
  BASLER_ACA2000 * This = (BASLER_ACA2000*)user_data;
  This->newImageCallback(type, buffer);
}


int BASLER_ACA2000::startAquisition(void )
{
  static unsigned int arv_option_packet_timeout = 20;
  static unsigned int arv_option_frame_retention = 100;
  static gboolean arv_option_auto_socket_buffer = FALSE;
static gboolean arv_option_no_packet_resend = TRUE;
  arv_camera_set_pixel_format(camera, 0x010c002a); ////Basler uses a colour coding not registered by standard in the aravis framework, but does exist in JAI as J_GVSP_PIX_BAYGR12_PACKED
  #pragma message "Using hardcoding for pixel format, monitor https://bugzilla.gnome.org/show_bug.cgi?id=687833 for when we can remove hardcoding"
  gint payload;
  payload = arv_camera_get_payload (camera);
  stream = arv_camera_create_stream (camera, &BASLER_ACA2000::newImageCallbackWrapper, (void*)this);
  if (stream != NULL) {
    if (ARV_IS_GV_STREAM (stream)) {
      if (arv_option_auto_socket_buffer)
	      g_object_set (stream,
			    "socket-buffer", ARV_GV_STREAM_SOCKET_BUFFER_AUTO,
			    "socket-buffer-size", 0,
			    NULL);
      if (arv_option_no_packet_resend)
	      g_object_set (stream,
			    "packet-resend", ARV_GV_STREAM_PACKET_RESEND_NEVER,
			    NULL);
      g_object_set (stream,
		    "packet-timeout", (unsigned) arv_option_packet_timeout * 1000,
		    "frame-retention", (unsigned) arv_option_frame_retention * 1000,
		    NULL);
      for (int i = 0; i < 16; i++)
	arv_stream_push_buffer (stream, arv_buffer_new (payload, NULL));

      arv_camera_set_acquisition_mode (camera, ARV_ACQUISITION_MODE_CONTINUOUS);
      arv_camera_start_acquisition (camera);
    }
}
}

void BASLER_ACA2000::newImageCallback(ArvStreamCallbackType type, ArvBuffer* buffer)
{

  if(type == ARV_STREAM_CALLBACK_TYPE_BUFFER_DONE)
  {
    this->bufferQue.enqueue(buffer);
    this->bufferSem.release(1);
  }
}

int BASLER_ACA2000::stopAquisition(void )
{

}

void BASLER_ACA2000::run()
{
  std::cout << "Basler_AVA2000 TID:" << syscall(SYS_gettid) << std::endl << std::flush;
  char * tmp = (char*)malloc(1000*2000*5);
  nFrames = 0;  
  successFrames = 0;
  failedFrames = 0;
  framePeriod.start();
  while(abort==false)
  {
    if(nFrames >=frameAvg)
    {
      nFrames = 0;
      float fps = framePeriod.elapsed();
      fps /= frameAvg;
      fps = 1000.0f/fps;
      emit(measuredFPS(fps));
      emit(measuredFrameStats(successFrames, failedFrames));
      framePeriod.restart();
    }
    else
    {
      nFrames++;
    }
    if(updateptimer)
    {
      setitimer(ITIMER_PROF, &ptimer, NULL);
      updateptimer = false;
    }
    bool cont = false;
    while(cont==false)
    {
      cont = this->bufferSem.tryAcquire(1,100);
      if(abort == true)
	return;
    }
    ArvBuffer * buffer = this->bufferQue.dequeue();
    if(buffer->status==ARV_BUFFER_STATUS_SUCCESS)
    {
      successFrames++;
 //     std::cout << "Buffer status SUCCESS" << std::endl;
      //See aravis-0.2.0/docs/reference/aravis/html/ArvBuffer.html
      switch (buffer->pixel_format)
      {
	//Mono
	case ARV_PIXEL_FORMAT_MONO_8:
	case ARV_PIXEL_FORMAT_MONO_8_SIGNED:
	  
	case ARV_PIXEL_FORMAT_MONO_10:
	case ARV_PIXEL_FORMAT_MONO_10_PACKED:
	  
	case ARV_PIXEL_FORMAT_MONO_12:
	case ARV_PIXEL_FORMAT_MONO_12_PACKED:
	  
	case ARV_PIXEL_FORMAT_MONO_14:
	  
	case ARV_PIXEL_FORMAT_MONO_16:
	  std::cout << "Not yet implemented mono" << std::endl;
	  break;
	  
	//Bayer
	case ARV_PIXEL_FORMAT_BAYER_GR_8:
	case ARV_PIXEL_FORMAT_BAYER_RG_8:
	case ARV_PIXEL_FORMAT_BAYER_GB_8:
	case ARV_PIXEL_FORMAT_BAYER_BG_8:
	case ARV_PIXEL_FORMAT_BAYER_GR_10:
	case ARV_PIXEL_FORMAT_BAYER_RG_10:
	case ARV_PIXEL_FORMAT_BAYER_GB_10:
	case ARV_PIXEL_FORMAT_BAYER_BG_10:
	case ARV_PIXEL_FORMAT_BAYER_GR_12:
	case ARV_PIXEL_FORMAT_BAYER_RG_12:
	case ARV_PIXEL_FORMAT_BAYER_GB_12:
	case ARV_PIXEL_FORMAT_BAYER_BG_12:
	  std::cout << "Not yet implemented bayer" << std::endl;
	  break;
	case ARV_PIXEL_FORMAT_BAYER_BG_12_PACKED:
	  {
	    cv::Mat unpacked;
//	    this->unpack12BitPacked(buffer, unpacked);
	    emit(this->newBayerBGImage(unpacked));
	  }
	  break;
	case 0x010c002a: //Basler uses a colour coding not registered by standard in the aravis framework, but does exist in JAI as J_GVSP_PIX_BAYGR12_PACKED
	{		 //Filed a bug as https://bugzilla.gnome.org/show_bug.cgi?id=687833
#pragma message "Using hardcoding for pixel format, monitor https://bugzilla.gnome.org/show_bug.cgi?id=687833 for when we can remove hardcoding"
	    
	    if(buffer->size >= ((buffer->width*buffer->height*3)/2))
	    {
	      this->unpack12BitPacked(buffer, tmp);
	      cv::Mat unpacked(buffer->height, buffer->width, cv::DataType<uint16_t>::type, (void*)tmp);
	      emit(this->newBayerGRImage(unpacked));	
	    }
	}
	break;
	//Color
	case ARV_PIXEL_FORMAT_RGB_8_PACKED:
	case ARV_PIXEL_FORMAT_BGR_8_PACKED:
	case ARV_PIXEL_FORMAT_RGBA_8_PACKED:
	case ARV_PIXEL_FORMAT_BGRA_8_PACKED:
	case ARV_PIXEL_FORMAT_RGB_10_PACKED:
	case ARV_PIXEL_FORMAT_BGR_10_PACKED:
	case ARV_PIXEL_FORMAT_RGB_12_PACKED:
	case ARV_PIXEL_FORMAT_BGR_12_PACKED:
	case ARV_PIXEL_FORMAT_YUV_411_PACKED:
	case ARV_PIXEL_FORMAT_YUV_422_PACKED:
	case ARV_PIXEL_FORMAT_YUV_444_PACKED:
	case ARV_PIXEL_FORMAT_RGB_8_PLANAR:
	case ARV_PIXEL_FORMAT_RGB_10_PLANAR:
	case ARV_PIXEL_FORMAT_RGB_12_PLANAR:
	case ARV_PIXEL_FORMAT_RGB_16_PLANAR:
	case ARV_PIXEL_FORMAT_YUV_422_YUYV_PACKED:
	  std::cout << "Not yet implemented color" << std::endl;
	  break;
	  
	//Custom
	case ARV_PIXEL_FORMAT_CUSTOM_BAYER_GR_12_PACKED:
	case ARV_PIXEL_FORMAT_CUSTOM_BAYER_RG_12_PACKED:
	case ARV_PIXEL_FORMAT_CUSTOM_BAYER_GB_12_PACKED:
	case ARV_PIXEL_FORMAT_CUSTOM_BAYER_BG_12_PACKED:
	case ARV_PIXEL_FORMAT_CUSTOM_YUV_422_YUYV_PACKED:
	case ARV_PIXEL_FORMAT_CUSTOM_BAYER_GR_16:
	case ARV_PIXEL_FORMAT_CUSTOM_BAYER_RG_16:
	case ARV_PIXEL_FORMAT_CUSTOM_BAYER_GB_16:
	case ARV_PIXEL_FORMAT_CUSTOM_BAYER_BG_16:
	  std::cout << "Not yet implemented custom" << std::endl;
	  break;
	default:
	  std::cout << "Unknown color coding :0x" << std::hex << buffer->pixel_format << std::dec << std::endl;
	  break;
      }
    }
    else
    {
      failedFrames++;
//       switch (buffer->status)
//       {
// 	case 	ARV_BUFFER_STATUS_SUCCESS:
// 	  std::cout << "Buffer status failed with status SUCCESS" << std::endl;
// 	  break;
// 	case ARV_BUFFER_STATUS_CLEARED:
// 	  std::cout << "Buffer status failed with status CLEARED" << std::endl;
// 	  break;
// 	case ARV_BUFFER_STATUS_TIMEOUT:
// 	  std::cout << "Buffer status failed with status TIMEOUT" << std::endl;
// 	  break;
// 	case ARV_BUFFER_STATUS_MISSING_PACKETS:
// 	  std::cout << "Buffer status failed with status MISSING PACKETS" << std::endl;
// 	  break;
// 	case ARV_BUFFER_STATUS_WRONG_PACKET_ID:
// 	  std::cout << "Buffer status failed with status Wrong PACKET ID" << std::endl;
// 	  break;
// 	case ARV_BUFFER_STATUS_SIZE_MISMATCH:
// 	  std::cout << "Buffer status failed with status SIZE MISMATCH" << std::endl;
// 	  break;
// 	case ARV_BUFFER_STATUS_FILLING:
// 	  std::cout << "Buffer status failed with status FILLING" << std::endl;
// 	  break;
// 	case ARV_BUFFER_STATUS_ABORTED:
// 	  std::cout << "Buffer status failed with status ABORTED" << std::endl;
// 	  break;
// 	default:
// 	  std::cout << "Buffer status failed with unknown error: 0x" << std::hex << buffer->status << std::dec << std::endl;
// 	  break;
//       }     
    }
   arv_stream_push_buffer (stream, buffer);   
  }
}
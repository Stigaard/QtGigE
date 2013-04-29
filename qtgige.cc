// Copyright (c) 2011 University of Southern Denmark. All rights reserved.
// Use of this source code is governed by the MIT license (see license.txt).
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <semaphore.h>
#include <QElapsedTimer>
#include <QDialog>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>
#include <qcheckbox.h>
#include <QDateTime>
#include <cstdio>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "qtgige.moc"
#include "qtgige.h"
#ifndef EMULATE_CAMERA
  #undef signals
  #include <arv.h>
#endif

QTGIGE::QTGIGE(char* deviceId)
{
#ifndef EMULATE_CAMERA
  this->camera = arv_camera_new (deviceId);
  if(this->camera == 0x0)
  {
    std::cout << "Failed to open camera with device id \"" << deviceId << "\"" << std::endl;
    assert(this->camera);
  }
#endif
  updateptimer = false;
//   std::cout << "Vendor name:" << arv_camera_get_vendor_name (camera) << std::endl;
//   std::cout << "Model name:" << arv_camera_get_model_name (camera) << std::endl;
//   std::cout << "Device ID:" << arv_camera_get_device_id (camera) << std::endl;
  
  abort = false;
  this->drawSettingsDialog();
#ifndef EMULATE_CAMERA
  dev = arv_camera_get_device(camera);
  genicam = arv_device_get_genicam(dev);
#else
  roi_width = 2046;
  roi_height = 1086;
  roi_x = 1587;
  roi_y = 0;
  roi_scale = 1.0;
  roi_cpos = 0;
#endif
  this->start();
}

void QTGIGE::setptimer(itimerval timer)
{
  setitimer(ITIMER_PROF, &timer, NULL	);
  ptimer = timer;
  updateptimer = true;
}

QTGIGE::~QTGIGE()
{
  abort = true;
  this->msleep(300);
#ifndef EMULATE_CAMERA
  g_object_unref(this->camera);
#endif
}

void QTGIGE::showCameraSettings(void )
{
  PrintParms();
}

void QTGIGE::drawSettingsDialog(void )
{
  settings = new QDialog();
  settings->hide();
  treeWidget = new QTreeWidget(settings);
  currentSetting = new QWidget(settings);
  treeWidget->setColumnCount(2);
  QStringList headers;
  headers.append(QString("Feature"));
  headers.append(QString("Value"));
  treeWidget->setHeaderLabels(headers);
  settings->setWindowTitle(QString("Camera settings"));
  settingsLayout = new QGridLayout(settings);
//  treeWidget->setMinimumSize(600, 800);
  treeWidget->setColumnWidth(0, 300);
  connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(newSettingSelected(QTreeWidgetItem*,int)));
  connect(settings, SIGNAL(finished(int)), treeWidget, SLOT(clear()));
  connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), treeWidget, SLOT(update()));
//  currentSetting->setMinimumSize(600,800);
  settingsLayout->addWidget(treeWidget,1,1);
  settingsLayout->addWidget(currentSetting, 1,2);
  treeWidget->setMinimumHeight(600);
  treeWidget->setMinimumWidth(500);
  settings->setLayout(settingsLayout);
  currentSettingLayout = new QGridLayout(currentSetting);
  currentSetting->setLayout(currentSettingLayout);
  settings->hide();
}


void QTGIGE::PrintParms(void )
{
#ifndef EMULATE_CAMERA
  gigE_list_features(genicam, "Root", true, treeWidget->invisibleRootItem());
#endif
  treeWidget->expandItem(treeWidget->topLevelItem(0));
  settings->show();
}

void QTGIGE::newSettingSelected(QTreeWidgetItem* item, int column)
{
  #ifndef EMULATE_CAMERA
    QLayoutItem *child;
    while ((child = currentSettingLayout->takeAt(0)) != 0) {
	child->widget()->disconnect();
	delete child->widget();
	delete child;
    }
    currentSetting->update();
    ArvGcNode *node;
    node = arv_gc_get_node (genicam, item->text(0).toLocal8Bit().constData());
    const char *description;
    description = arv_gc_feature_node_get_description (ARV_GC_FEATURE_NODE (node), NULL);
    QLabel * descriptionLabel = new QLabel(currentSetting);
    descriptionLabel->setText(description);
    descriptionLabel->setWordWrap(true);
    
    QLabel * text = new QLabel(currentSetting);
    QLineEdit * codeSnippet = new QLineEdit(currentSetting);
    codeSnippet->setReadOnly(true);
    text->setText(item->text(0));
    currentSettingLayout->addWidget(text, 1,1);
    currentSettingLayout->addWidget(descriptionLabel, 3,1);
    QComboBox *comboValue = new QComboBox(currentSetting);
    QSlider * sliderValue = new QSlider(currentSetting);
    QCheckBox *chkboxValue = new QCheckBox(currentSetting);
    QPushButton *actionButton = new QPushButton(currentSetting);
    //arv_dom_get_node_name(node);
    QString nodeType(arv_dom_node_get_node_name(ARV_DOM_NODE(node)));
    std::cout << "NodeType is:" << nodeType.toLocal8Bit().constData() << std::endl;
    if(nodeType == "Category")
      {
	std::cout << "is category" << std::endl;
	//Unhandled
      } 
    else if(nodeType == "Enumeration")
      {
	const GSList *childs;
	const GSList *iter;
	QString selectedEnumNode = arv_gc_enumeration_get_string_value(ARV_GC_ENUMERATION (node), NULL);
	QString nodeName(arv_gc_feature_node_get_name(ARV_GC_FEATURE_NODE (node)));
	comboValue->setProperty("nodeName", nodeName);
	comboValue->setProperty("nodeItem", QVariant::fromValue((void*)item));
	comboValue->setProperty("codeSnippet", QVariant::fromValue((void*)codeSnippet));
	//value->setProperty("nodeItem", *item);
	childs = arv_gc_enumeration_get_entries (ARV_GC_ENUMERATION (node));
	int selectedIndex = 0;
	int currentIndex = 0;
	for (iter = childs; iter != NULL; iter = iter->next) 
	  {
	    if (arv_gc_feature_node_is_implemented ((ArvGcFeatureNode*)iter->data, NULL)) 
	    {
	      if(arv_gc_feature_node_is_available ((ArvGcFeatureNode*)iter->data, NULL))
	      {
		QString enumNode = arv_gc_feature_node_get_name((ArvGcFeatureNode*)iter->data);
		comboValue->addItem(enumNode);
		if(enumNode.compare(selectedEnumNode)==0)
		{
		  selectedIndex = currentIndex;
		}
		currentIndex++;
	      }
	    }
	  }
	codeSnippet->setText(QString("QTGIGE::writeEnum(\"") + nodeName + QString("\", \"") + comboValue->itemText(selectedIndex) + QString("\");"));
	comboValue->setCurrentIndex(selectedIndex);
	currentSettingLayout->addWidget(codeSnippet, 4,1);
	currentSettingLayout->addWidget(comboValue,2,1);
      }
    else if(nodeType == "Command")
      {
	actionButton->setProperty("nodeItem", QVariant::fromValue((void*)item));
	actionButton->setText(item->text(0));
	currentSettingLayout->addWidget(actionButton,2,1);
	currentSettingLayout->addWidget(codeSnippet, 4,1);
	codeSnippet->setText(QString("QTGIGE::emitAction(\"") + item->text(0) + QString("\");"));
	connect(actionButton, SIGNAL(clicked(bool)), this, SLOT(emitActionFromSettings()));
      //unhandled 
      }
    else if(nodeType == "StringReg")
      {
	//unhandled
      }
    else if(nodeType == "Float")
      {
	//Aravis Seems to give invalid values for floats...
	GError *err=NULL;
	double val = arv_gc_float_get_value(ARV_GC_FLOAT(node), &err);
	if (err != NULL)
	{
	  /* Report error to user, and free error */
	  std::cerr << "Error in getting float value:" << err->message << std::endl;
	  g_error_free (err);
	}
	err = NULL;
	QString unit(arv_gc_float_get_unit(ARV_GC_FLOAT(node), &err));
	if (err != NULL)
	{
	  /* Report error to user, and free error */
	  std::cerr << "Error in getting float unit:" << err->message << std::endl;
	  g_error_free (err);
	}
	err = NULL;
	double minVal = arv_gc_float_get_min(ARV_GC_FLOAT(node), &err);
	if (err != NULL)
	{
	  /* Report error to user, and free error */
	  std::cerr << "Error in getting float minimum:" << err->message << std::endl;
	  g_error_free (err);
	}
	err = NULL;
	double maxVal = arv_gc_float_get_max(ARV_GC_FLOAT(node), &err);
	if (err != NULL)
	{
	  /* Report error to user, and free error */
	  std::cerr << "Error in getting float maximum:" << err->message << std::endl;
	  g_error_free (err);
	}
	err = NULL;
	double incr = arv_gc_float_get_inc(ARV_GC_FLOAT(node), &err);
	if (err != NULL)
	{
	  /* Report error to user, and free error */
	  std::cerr << "Error in getting float increment:" << err->message << std::endl;
	  g_error_free (err);
	}
	err = NULL;
	double mul = 1.0f/incr;
	std::cout << "min:" << minVal << " max:" << maxVal << " val:" << val << " incr:" << incr << " mul:" << mul << std::endl;
	sliderValue->setMinimum(minVal*mul);
	sliderValue->setMaximum(maxVal*mul);
	sliderValue->setTickInterval(incr*mul);
	sliderValue->setValue(val*mul);
	sliderValue->setProperty("multiplier", mul);
	sliderValue->setProperty("codeSnippet", QVariant::fromValue((void*)codeSnippet));
	sliderValue->setProperty("nodeItem", QVariant::fromValue((void*)item));
	currentSettingLayout->addWidget(sliderValue,2,1);
	connect(sliderValue, SIGNAL(valueChanged(int)), this, SLOT(writeFloatFromSettings(int)));
      } 
    else if(nodeType == "Boolean")
      {
	bool val = arv_gc_boolean_get_value(ARV_GC_BOOLEAN(node), NULL);
	if(val)
	  chkboxValue->setCheckState(Qt::Checked);
	else
	  chkboxValue->setCheckState(Qt::Unchecked);
	chkboxValue->setText(item->text(0));
	chkboxValue->setProperty("codeSnippet", QVariant::fromValue((void*)codeSnippet));
	chkboxValue->setProperty("nodeItem", QVariant::fromValue((void*)item));
	currentSettingLayout->addWidget(chkboxValue,2,1);
	currentSettingLayout->addWidget(codeSnippet, 4,1);
	if(val)
	  codeSnippet->setText(QString("QTGIGE::writeBool(\"") + item->text(0) + QString("\", true);"));
	else
	  codeSnippet->setText(QString("QTGIGE::writeBool(\"") + item->text(0) + QString("\", false);"));
	connect(chkboxValue, SIGNAL(stateChanged(int)), this, SLOT(writeBoolFromSettings(int)));
      }
    else if(nodeType == "Integer")
      {
	int incr  = arv_gc_integer_get_inc(ARV_GC_INTEGER(node), NULL);
	int val  = arv_gc_integer_get_value(ARV_GC_INTEGER(node), NULL);
	int minVal  = arv_gc_integer_get_min(ARV_GC_INTEGER(node), NULL);
	int maxVal  = arv_gc_integer_get_max(ARV_GC_INTEGER(node), NULL);
	QString unit(arv_gc_integer_get_unit(ARV_GC_INTEGER(node), NULL));
	std::cout << "min:" << minVal << " max:" << maxVal << " val:" << val << " incr:" << incr << " mul:" << 1 << std::endl;
	sliderValue->setMinimum(minVal);
	sliderValue->setMaximum(maxVal);
	sliderValue->setTickInterval(incr);
	sliderValue->setValue(val);
	sliderValue->setProperty("multiplier", 1);
	sliderValue->setProperty("codeSnippet", QVariant::fromValue((void*)codeSnippet));
	sliderValue->setProperty("nodeItem", QVariant::fromValue((void*)item));
	sliderValue->setOrientation(Qt::Horizontal);
	currentSettingLayout->addWidget(sliderValue,2,1);
	currentSettingLayout->addWidget(codeSnippet, 4,1);
	codeSnippet->setText(QString("QTGIGE::writeInt(\"") + item->text(0) + QString("\", ") + QString::number(val) + QString(");"));
	connect(sliderValue, SIGNAL(valueChanged(int)), this, SLOT(writeIntFromSettings(int)));
      }
    //else unhandled
    currentSetting->setVisible(true);
    settings->adjustSize();
    currentSetting->setMinimumWidth(400);
    connect(comboValue, SIGNAL(currentIndexChanged(QString)), this, SLOT(writeEnumFromSettingsSelectorMapper(QString)));
#endif //#ifndef EMULATE_CAMERA
}

void QTGIGE::writeInt(QString nodeName, int value)
{
  std::cout << "Received request to set " << nodeName.toLocal8Bit().constData() << " to " << value << std::endl;
#ifndef EMULATE_CAMERA
  ArvGcNode *node = arv_gc_get_node (genicam, nodeName.toLocal8Bit().constData());
  arv_gc_integer_set_value(ARV_GC_INTEGER(node), value, NULL);
#endif
}

void QTGIGE::writeBool(QString nodeName, bool value)
{
  std::cout << "Received request to set " << nodeName.toLocal8Bit().constData() << " to " << value << std::endl;
#ifndef EMULATE_CAMERA
  ArvGcNode *node = arv_gc_get_node (genicam, nodeName.toLocal8Bit().constData());
  arv_gc_boolean_set_value(ARV_GC_BOOLEAN(node), value, NULL);
#endif
}

void QTGIGE::emitAction(QString nodeName)
{
  std::cout << "Received request to action " << nodeName.toLocal8Bit().constData() << std::endl;
#ifndef EMULATE_CAMERA
  ArvGcNode *node = arv_gc_get_node (genicam, nodeName.toLocal8Bit().constData());
  arv_gc_command_execute(ARV_GC_COMMAND(node),NULL);
#endif
}

void QTGIGE::emitActionFromSettings(void )
{
  QTreeWidgetItem * item = (QTreeWidgetItem *) this->sender()->property("nodeItem").value<void *>();
  QString nodeName = item->text(0);
  emitAction(nodeName);
}


void QTGIGE::writeBoolFromSettings(int value)
{
  QTreeWidgetItem * item = (QTreeWidgetItem *) this->sender()->property("nodeItem").value<void *>();
  QString nodeName = item->text(0);
  QLineEdit * codeSnippet = (QLineEdit *) this->sender()->property("codeSnippet").value<void *>();
  if(value==Qt::Checked)
  {
    item->setText(1, "True");
    codeSnippet->setText(QString("QTGIGE::writeBool(\"") + item->text(0) + QString("\", true);"));
    writeBool(nodeName, true);
  }
  else
  {
    item->setText(1, "False");
    codeSnippet->setText(QString("QTGIGE::writeBool(\"") + item->text(0) + QString("\", false);"));
    writeBool(nodeName, false);
  }
}


void QTGIGE::writeIntFromSettings(int value)
{
  QTreeWidgetItem * item = (QTreeWidgetItem *) this->sender()->property("nodeItem").value<void *>();
  QString nodeName = item->text(0);
  QLineEdit * codeSnippet = (QLineEdit *) this->sender()->property("codeSnippet").value<void *>();
  item->setText(1, QString::number((((float)value))));
  codeSnippet->setText(QString("QTGIGE::writeInt(\"") + item->text(0) + QString("\", ") + QString::number(value) + QString(");"));
  writeInt(nodeName, value);
}


void QTGIGE::writeFloat(QString nodeName, float value)
{
  std::cout << "Received request to set " << nodeName.toLocal8Bit().constData() << " to " << value << std::endl;
#ifndef EMULATE_CAMERA
  ArvGcNode *node = arv_gc_get_node (genicam, nodeName.toLocal8Bit().constData());
  arv_gc_float_set_value(ARV_GC_FLOAT(node), value, NULL);
#endif
}

void QTGIGE::writeFloatFromSettings(int value)
{
  QTreeWidgetItem * item = (QTreeWidgetItem *) this->sender()->property("nodeItem").value<void *>();
  QString nodeName = item->text(0);
  QLineEdit * codeSnippet = (QLineEdit *) this->sender()->property("codeSnippet").value<void *>();
  float mul = this->sender()->property("multiplier").toFloat();
  item->setText(1, QString::number((((float)value)/mul)));
  writeFloat(nodeName, (((float)value)/mul));
}


void QTGIGE::writeEnumFromSettingsSelectorMapper(QString value)
{
  QString nodeName(this->sender()->property("nodeName").toString());
  QTreeWidgetItem * item = (QTreeWidgetItem *) this->sender()->property("nodeItem").value<void *>();
  QLineEdit * codeSnippet = (QLineEdit *) this->sender()->property("codeSnippet").value<void *>();
  codeSnippet->setText(QString("QTGIGE::writeEnum(\"") + nodeName + QString("\", \"") + value + QString("\");"));
  item->setText(1, value);
  writeEnum(nodeName, value);
}


void QTGIGE::writeEnum(QString nodeName, QString value)
{
  std::cout << "Received request to set " << nodeName.toLocal8Bit().constData() << " to " << value.toLocal8Bit().constData() << std::endl;
#ifndef EMULATE_CAMERA
  ArvGcNode *node = arv_gc_get_node (genicam, nodeName.toLocal8Bit().constData());
  arv_gc_enumeration_set_string_value(ARV_GC_ENUMERATION(node), value.toLocal8Bit().constData(), NULL);
#endif
}

#ifndef EMULATE_CAMERA
void QTGIGE::gigE_list_features(ArvGc* genicam, const char* feature, gboolean show_description, QTreeWidgetItem * parent)
{
	ArvGcNode *node;
	node = arv_gc_get_node (genicam, feature);
	
	if (ARV_IS_GC_FEATURE_NODE (node) &&
	    arv_gc_feature_node_is_implemented (ARV_GC_FEATURE_NODE (node), NULL)) 
	{
		if(arv_gc_feature_node_is_available (ARV_GC_FEATURE_NODE (node), NULL))
		{
		  int i;
		  QTreeWidgetItem *item = new QTreeWidgetItem(parent);
		  QString nodeType(arv_dom_node_get_node_name(ARV_DOM_NODE(node)));
		  item->setText(0,QString(feature));
		
		  const char *description;
		  description = arv_gc_feature_node_get_description (ARV_GC_FEATURE_NODE (node), NULL);
		  if (description)
		  {
			  item->setToolTip(0, description);
			  item->setToolTip(1, description);
		  }
		  if (nodeType == "Category") 
		  {
			  const GSList *features;
			  const GSList *iter;

			  features = arv_gc_category_get_features (ARV_GC_CATEGORY (node));

			  for (iter = features; iter != NULL; iter = iter->next)
				  gigE_list_features (genicam, (char*)(iter->data), show_description, item);
		  } 
		  else if (nodeType == "Enumeration") 
		  {
			  const GSList *childs;
			  const GSList *iter;
			  childs = arv_gc_enumeration_get_entries (ARV_GC_ENUMERATION (node));
			  item->setText(1, arv_gc_enumeration_get_string_value(ARV_GC_ENUMERATION (node), NULL));
		  } 
		  else if(nodeType == "Float")
		  {
		    double val = arv_gc_float_get_value (ARV_GC_FLOAT(node), NULL);
		    const char * unit = arv_gc_float_get_unit(ARV_GC_FLOAT(node),NULL);
		    item->setText(1, QString(QString::number(val) + QString(unit)));
		  } 
		  else if(nodeType == "Integer")
		  {
		    qint64 val = arv_gc_integer_get_value(ARV_GC_INTEGER(node), NULL);
		    const char * unit = arv_gc_integer_get_unit(ARV_GC_INTEGER(node), NULL);
		    item->setText(1, QString(QString::number(val) + QString(unit)));
		  } 
		  else if(nodeType == "Command")
		  {
		    item->setText(1, QString(" Action "));
		  } else if(nodeType == "StringReg")
		  {
		    const char * str = arv_gc_string_get_value(ARV_GC_STRING(node),NULL);
		    item->setText(1, QString(str));
		  }
		  else if(nodeType == "Boolean")
		  {
		    bool val = arv_gc_boolean_get_value(ARV_GC_BOOLEAN(node),NULL);
		    if(val)
		      item->setText(1,QString("True"));
		    else
		      item->setText(1,QString("False"));
		  } else
		  {
		    item->setText(1,QString("Unhandled"));
		  }
		}
	}
}
#endif //#ifndef EMULATE_CAMERA

int64 QTGIGE::getSensorWidth()
{
    QString nodeName = "WidthMax";
#ifndef EMULATE_CAMERA
    ArvGcNode *node = arv_gc_get_node (genicam, nodeName.toLocal8Bit().constData());
    int64 sensorWidth = arv_gc_integer_get_value(ARV_GC_INTEGER(node), NULL);
    return sensorWidth;
#else
    return -1;
#endif
}

int64 QTGIGE::getSensorHeight()
{
    QString nodeName = "HeightMax";
#ifndef EMULATE_CAMERA
    ArvGcNode *node = arv_gc_get_node (genicam, nodeName.toLocal8Bit().constData());
    int64 sensorHeight = arv_gc_integer_get_value(ARV_GC_INTEGER(node), NULL);
    return sensorHeight;
#else
    return -1;
#endif
}

int QTGIGE::setROI(int x, int y, int width, int height)
{
#ifndef EMULATE_CAMERA
    int64 sensorHeight = getSensorHeight();
    int64 sensorWidth = getSensorWidth();
    
    assert(0 <= x);
    assert(0 <= y);
    assert(x + width <= sensorWidth);
    assert(y + height <= sensorHeight);
    
    arv_camera_set_region (camera, x, y, width, height);
#endif
}

int QTGIGE::setExposure(float period)
{
#ifndef EMULATE_CAMERA
  arv_camera_set_exposure_time (camera, period);
#endif
}

int QTGIGE::setGain(float gain)
{
#ifndef EMULATE_CAMERA
  arv_camera_set_gain (camera, gain);
#endif
}

#ifndef EMULATE_CAMERA
void QTGIGE::unpack12BitPacked(const ArvBuffer* img, char* unpacked16)
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
#endif //#ifndef EMULATE_CAMERA

void QTGIGE::convert16to8bit(cv::InputArray in, cv::OutputArray out)
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

#ifndef EMULATE_CAMERA
void QTGIGE::newImageCallbackWrapper(void* user_data, ArvStreamCallbackType type, ArvBuffer* buffer)
{
  //Please note that this is in fact a static method, where we pass the instance as a user_data parameter,
  //therefore all ops should be performed on the This object, instead of the this (which shouldn't exist)
  QTGIGE * This = (QTGIGE*)user_data;
  This->newImageCallback(type, buffer);
}
#endif //#ifndef EMULATE_CAMERA


int QTGIGE::startAquisition(void )
{
#ifndef EMULATE_CAMERA
  static unsigned int arv_option_packet_timeout = 40;
  static unsigned int arv_option_frame_retention = 200;
  static gboolean arv_option_auto_socket_buffer = FALSE;
  static gboolean arv_option_no_packet_resend = TRUE;
  arv_camera_set_pixel_format(camera, ARV_PIXEL_FORMAT_BAYER_GR_12_PACKED);
  gint payload;
  payload = arv_camera_get_payload (camera);
  stream = arv_camera_create_stream (camera, &QTGIGE::newImageCallbackWrapper, (void*)this);
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
#endif //#ifndef EMULATE_CAMERA
}

#ifndef EMULATE_CAMERA
void QTGIGE::newImageCallback(ArvStreamCallbackType type, ArvBuffer* buffer)
{
  if(type == ARV_STREAM_CALLBACK_TYPE_BUFFER_DONE)
  {
    //Calibrate clk offset
    if(offset<0)
    {
      qint64 now = QDateTime::currentMSecsSinceEpoch();
      offset = now - ((qint64)(buffer->timestamp_ns/1000000));
    }
    this->bufferQue.enqueue(buffer);
    this->bufferSem.release(1);
  }
}
#endif //#ifndef EMULATE_CAMERA

int QTGIGE::stopAquisition(void )
{

}

void QTGIGE::run()
{
  std::cout << "Basler_AVA2000 TID:" << syscall(SYS_gettid) << std::endl << std::flush;
  char * tmp= (char*)malloc(1000*2000*5);
//  posix_memalign((void**)(&tmp),8,1000*2000*5); //alloc aligned memmory
  nFrames = 0;  
  successFrames = 0;
  failedFrames = 0;
  
#ifndef EMULATE_CAMERA
  offset = -1;
#else
  cv::Mat emu_image;
  std::cout << "Using " << EMULATION_INPUT_FILE << " as input file for emulation" << std::endl;
  emu_image = cv::imread(EMULATION_INPUT_FILE, cv::IMREAD_GRAYSCALE);
  cv::transpose(emu_image, emu_image);
  std::cout << "Emulation image size " << emu_image.size().width << "x" << emu_image.size().height << "x" << emu_image.channels() << std::endl;
  unsigned int length = emu_image.size().height;
#endif
  
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
#ifndef EMULATE_CAMERA
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
	case ARV_PIXEL_FORMAT_BAYER_BG_12_PACKED:
	  std::cout << "Not yet implemented bayer" << std::endl;
	  break;
	case ARV_PIXEL_FORMAT_BAYER_GR_12_PACKED: 
	{	
	    if(buffer->size >= ((buffer->width*buffer->height*3)/2))
	    {
	      this->unpack12BitPacked(buffer, tmp);
	      cv::Mat unpacked(buffer->height, buffer->width, cv::DataType<uint16_t>::type, (void*)tmp);
	      qint64 timestamp_in_us = (buffer->timestamp_ns)/1000 + (offset*1000);
	      emit(this->newBayerGRImage(unpacked, timestamp_in_us));	
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
#else //#ifndef EMULATE_CAMERA
   //Send out next emulated frame
   roi_cpos -= roi_height/10;
   if(roi_cpos-roi_height<0)
     roi_cpos = length;
   cv::Mat subImg = emu_image(cv::Range(roi_cpos-roi_height, roi_cpos), cv::Range(roi_x, roi_x+roi_width));
//   cv::Mat subImg16;
//   convert8to16bit(subImg, subImg16);
  cv::Mat RGB161616(roi_height,roi_width, cv::DataType<uint16_t>::type);
  subImg.convertTo(RGB161616, RGB161616.type(), 256.0);
  //Set red and blue in bayer pattern = 0
  uint16_t h = RGB161616.size().height;
  uint16_t w = RGB161616.size().width;
  uint16_t* ptr = (uint16_t*)RGB161616.ptr();
  for(uint16_t y = 0; y < h; y+=2)
  {
    for(uint16_t x = 1; x < w; x+=2)
    {
      ptr[y*w+x] = 0;
    }
  }
  for(uint16_t y = 1; y < h; y+=2)
  {
    for(uint16_t x = 0; x < w; x+=2)
    {
      ptr[y*w+x] = 0;
    }
  }
  //  std::cout << "RGB161616 image size " << RGB161616.size().width << "x" << RGB161616.size().height << "x" << RGB161616.channels() << std::endl;
  //cv::imwrite("test.png", RGB161616);
   //emit(this->newBayerGRImage(RGB161616, QDateTime::currentMSecsSinceEpoch()*1000));
   emit(this->newBayerGRImage(RGB161616, roi_cpos));
   this->msleep(300);
#endif //#ifndef EMULATE_CAMERA
  }
}

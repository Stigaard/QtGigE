#include "cqtopencvviewergl.h"
#include "cqtopencvviewergl.moc"
#include <QTimer>

CQtOpenCVViewerGl::CQtOpenCVViewerGl(QWidget *parent) :
    QGLWidget(parent)
{
     mBgColor = QColor::fromRgb(0, 0, 150);
}

void CQtOpenCVViewerGl::initializeGL()
{
    makeCurrent();
    //Adjust the viewport
    glViewport(0,0,this->width(), this->height());
    
    //Adjust the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-this->width()/2, this->width()/2, this->height()/2, -this->height()/2, -1, 1);	
    qglClearColor(mBgColor.darker());
}

void CQtOpenCVViewerGl::resizeGL(int width, int height)
{
    makeCurrent();
    //Adjust the viewport
    glViewport(0,0,this->width(), this->height());
    
    //Adjust the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-this->width()/2, this->width()/2, this->height()/2, -this->height()/2, -1, 1);	
    qglClearColor(mBgColor.darker());
}

void CQtOpenCVViewerGl::paintGL()
{
	float wzoom, hzoom, zoom;
	wzoom = (float)(this->width()) / (float)(mOrigImage.size().width);
	hzoom = (float)(this->height()) / (float)(mOrigImage.size().height);
	if(wzoom > hzoom)
	  zoom = hzoom;
	else
	  zoom = wzoom;
	//Clear the color buffer
	glClear(GL_COLOR_BUFFER_BIT);

	//Set the raster position
	glRasterPos2i(-this->width()/2,-this->height()/2);
	
	//Invert the image (the data coming from OpenCV is inverted)
	glPixelZoom(zoom,-zoom);

	//Draw image from OpenCV capture
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	bufferMutex.lock();
	glDrawPixels(mOrigImage.size().width, mOrigImage.size().height, GL_RGB, GL_UNSIGNED_BYTE,mOrigImage.ptr());		
	bufferMutex.unlock();
}

bool CQtOpenCVViewerGl::showImage( cv::Mat image )
{
    bufferMutex.lock();
    image.copyTo(mOrigImage);
    bufferMutex.unlock();
    glDraw();
    return true;
}

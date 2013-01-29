cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMAKE_Modules/")
find_package(Qt4 REQUIRED QtXml QtGui QtCore)
find_package(OpenCV REQUIRED)
find_package(GLIB2 REQUIRED)
find_package(GOBJECT REQUIRED)
find_package(PkgConfig REQUIRED)
find_library(PTHREAD_LIB pthread)

pkg_check_modules(ARAVIS aravis-0.2)
find_library(ARAVIS_LIBS ${ARAVIS_LIBRARIES} ${ARAVIS_LIBRARY_DIRS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

#Semi hardcoded the aravis libpath for now, should not do this, but is just for preliminary testing
set(ARAVIS_LIBS "${ARAVIS_LIBRARY_DIRS}/libaravis-0.2.so")

include_directories(${QT_INCLUDES} 
		    ${CMAKE_CURRENT_BINARY_DIR}
		    ${OPENCV_INCLUDE_DIR}
		    ${ARAVIS_INCLUDE_DIRS}
		    )
		    
add_definitions(${QT_DEFINITIONS})

set(QtGigE_SRCS qtgige.cc)

qt4_automoc(${QtGigE_SRCS})

ADD_LIBRARY(QtGigE STATIC ${QtGigE_SRCS})
	   
TARGET_LINK_LIBRARIES(QtGigE ${OpenCV_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${PTHREAD_LIB} ${ARAVIS_LIBS} ${GLIB2_LIBRARIES} ${GOBJECT_LIBRARIES})
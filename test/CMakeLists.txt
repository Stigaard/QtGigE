project(cam_interface_test)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../CMAKE_Modules/")
set(PROFILE 1) #If enabling profiling
set(ASMOUT 0) #If enabling assembly output for debugging


find_package(Qt4 REQUIRED QtXml QtGui QtCore QtOpenGL)
find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(RT REQUIRED)
find_package(GLIB2 REQUIRED)
find_package(GOBJECT REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(X11)

pkg_check_modules(ARAVIS aravis-0.2)
find_library(ARAVIS_LIBS ${ARAVIS_LIBRARIES} ${ARAVIS_LIBRARY_DIRS})
#Semi hardcoded the aravis libpath for now, should not do this, but is just for preliminary testing
set(ARAVIS_LIBS "${ARAVIS_LIBRARY_DIRS}/libaravis-0.2.so")

find_library(GL_LIB GL)
find_library(PTHREAD_LIB pthread)

#ASM output
if(ASMOUT)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -march=native -fverbose-asm")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg -fverbose-asm")
endif(ASMOUT)

#Performance tuning
if(PROFILE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -march=native")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
endif(PROFILE)

include_directories(${QT_INCLUDES} 
		    ${CMAKE_CURRENT_BINARY_DIR}
		    ${OPENCV_INCLUDE_DIR}
		    ${QT_QTOPENGL_INCLUDE_DIR}
		    ${ARAVIS_INCLUDE_DIRS}
		    ${X11_INCLUDE_DIRS}
		    )

set(cam_interface_test_SRCS demosaic_cv.cpp cam_interface_test.cpp main.cpp ../qtgige.cc cqtopencvviewergl.cpp)
qt4_automoc(${cam_interface_test_SRCS})
add_definitions(${QT_DEFINITIONS})
add_executable(cam_interface_test ${cam_interface_test_SRCS})

target_link_libraries(cam_interface_test 
		      ${QT_QTCORE_LIBRARY} 
		      ${QT_QTGUI_LIBRARY} 
		      ${QT_QTXML_LIBRARY}
		      ${QT_QTOPENGL_LIBRARY}
		      ${OpenCV_LIBS}
		      ${OPENGL_glu_LIBRARY}
		      ${RT_LIBRARIES}
		      ${GL_LIB}
		      ${PTHREAD_LIB}
		      ${ARAVIS_LIBS}
		      ${GLIB2_LIBRARIES}
		      ${GOBJECT_LIBRARIES}
		      ${X11_LIBRARIES}
		      )

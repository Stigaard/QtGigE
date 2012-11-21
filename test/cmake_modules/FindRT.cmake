#############################################################################
#
# $Id: FindRT.cmake 3057 2011-02-11 13:17:26Z fspindle $
#
# This file is part of the ViSP software.
# Copyright (C) 2005 - 2011 by INRIA. All rights reserved.
# 
# This software is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# ("GPL") version 2 as published by the Free Software Foundation.
# See the file LICENSE.txt at the root directory of this source
# distribution for additional information about the GNU GPL.
#
# For using ViSP with software that can not be combined with the GNU
# GPL, please contact INRIA about acquiring a ViSP Professional 
# Edition License.
#
# See http://www.irisa.fr/lagadic/visp/visp.html for more information.
# 
# This software was developed at:
# INRIA Rennes - Bretagne Atlantique
# Campus Universitaire de Beaulieu
# 35042 Rennes Cedex
# France
# http://www.irisa.fr/lagadic
#
# If you have questions regarding the use of this file, please contact
# INRIA at visp@inria.fr
# 
# This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
# WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
# Description:
# Try to find RT library material 
#
# RT_FOUND
# RT_LIBRARIES
#
# Authors:
# Fabien Spindler
#
#############################################################################

IF(NOT UNIX)
  # MESSAGE("FindRT.cmake: only available for Unix.")
  SET(RT_FOUND FALSE)
ELSE(NOT UNIX)
    
  FIND_LIBRARY(RT_LIBRARY
    NAMES rt
    PATHS 
    $ENV{RT_HOME}/lib
    /usr/lib
    /usr/local/lib
    )

  #MESSAGE("DBG RT_LIBRARY=${RT_LIBRARY}")
  
  ## --------------------------------
  
  IF(RT_LIBRARY)
    SET(RT_LIBRARIES ${RT_LIBRARY})
    SET(RT_FOUND TRUE)
  ELSE(RT_LIBRARY)
    SET(RT_FOUND FALSE)
    #MESSAGE(SEND_ERROR "rt library not found.")
  ENDIF(RT_LIBRARY)
  
  MARK_AS_ADVANCED(
    RT_LIBRARIES
    RT_LIBRARY
    )
ENDIF(NOT UNIX)

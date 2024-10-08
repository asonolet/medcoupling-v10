# Copyright (C) 2024  CEA, EDF
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

FIND_PACKAGE(SWIG REQUIRED)
INCLUDE(${SWIG_USE_FILE})

ADD_DEFINITIONS(${PYTHON_DEFINITIONS})

SET_SOURCE_FILES_PROPERTIES(ShapeRecogn.i PROPERTIES CPLUSPLUS ON)
IF ("${PYTHON_VERSION_MAJOR}" STREQUAL "3")
  SET_SOURCE_FILES_PROPERTIES(ShapeRecogn.i PROPERTIES SWIG_FLAGS "-py3")
ELSE()
  SET_SOURCE_FILES_PROPERTIES(ShapeRecogn.i PROPERTIES SWIG_DEFINITIONS "-shadow")
ENDIF()

SET(SWIG_MODULE_ShapeRecogn_EXTRA_FLAGS "")
IF(MEDCOUPLING_USE_64BIT_IDS)
  STRING(APPEND SWIG_MODULE_ShapeRecogn_EXTRA_FLAGS "-DMEDCOUPLING_USE_64BIT_IDS")
ENDIF(MEDCOUPLING_USE_64BIT_IDS)

SET (ShapeRecogn_SWIG_DPYS_FILES
  ShapeRecogn.i
)

INCLUDE_DIRECTORIES(
  ${PYTHON_INCLUDE_DIRS}
  ${MEDFILE_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../../MEDLoader
  ${CMAKE_CURRENT_SOURCE_DIR}/../../MEDCoupling_Swig
  ${CMAKE_CURRENT_SOURCE_DIR}/../../MEDCoupling
  ${CMAKE_CURRENT_SOURCE_DIR}/../../INTERP_KERNEL
  ${CMAKE_CURRENT_SOURCE_DIR}/../../INTERP_KERNEL/Bases
)

SET (SWIG_MODULE_ShapeRecogn_EXTRA_DEPS ${ShapeRecogn_SWIG_DPYS_FILES}
    ${shaperecogn_HEADERS_HXX}
    ${medcoupling_HEADERS_HXX} ${medcoupling_HEADERS_TXX}
    ${interpkernel_HEADERS_HXX} ${interpkernel_HEADERS_TXX})

IF(WIN32)
  SET_PROPERTY(SOURCE ShapeRecogn.i PROPERTY COMPILE_DEFINITIONS WIN32)
ENDIF()

IF(${CMAKE_VERSION} VERSION_LESS "3.8.0")   
  SWIG_ADD_MODULE(ShapeRecogn python ShapeRecogn.i)
ELSE()
  SWIG_ADD_LIBRARY(ShapeRecogn LANGUAGE python SOURCES ShapeRecogn.i)
ENDIF()

SWIG_LINK_LIBRARIES(ShapeRecogn ${PYTHON_LIBRARIES} ${PLATFORM_LIBS} shaperecogn medloader medcouplingcpp)
SWIG_CHECK_GENERATION(ShapeRecogn)
IF(WIN32)
  SET_TARGET_PROPERTIES(_ShapeRecogn PROPERTIES DEBUG_OUTPUT_NAME _ShapeRecogn_d)
  # To increase the size of the .obj file on Windows because ShapeRecognPYTHON_wrap.cxx, generated by SWIG, is too big
  TARGET_COMPILE_OPTIONS(_ShapeRecogn PRIVATE /bigobj) 
ENDIF(WIN32)

INSTALL(TARGETS _ShapeRecogn DESTINATION ${MEDCOUPLING_INSTALL_PYTHON})

SALOME_INSTALL_SCRIPTS(
  ${CMAKE_CURRENT_BINARY_DIR}/ShapeRecogn.py 
  ${MEDCOUPLING_INSTALL_PYTHON} 
  EXTRA_DPYS "${SWIG_MODULE_ShapeRecogn_REAL_NAME}")

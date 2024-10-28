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

if(UNIX)
  set(CBLAS_ROOT_DIR $ENV{CBLAS_ROOT_DIR} CACHE PATH "Path to the LAPACK/CBLAS.")
  if(CBLAS_ROOT_DIR)
    list(APPEND CMAKE_PREFIX_PATH "${CBLAS_ROOT_DIR}")
    find_package(CBLAS REQUIRED)
  else()
    find_package(BLAS REQUIRED)
  endif()
  set(LAPACK_ROOT_DIR $ENV{LAPACK_ROOT_DIR} CACHE PATH "Path to the LAPACKE")
  if(LAPACK_ROOT_DIR)
    set(LAPACK_ROOT_DIR $ENV{LAPACK_ROOT_DIR} CACHE PATH "Path to the LAPACK.")
    list(APPEND CMAKE_PREFIX_PATH "${LAPACK_ROOT_DIR}")
  endif()
  find_package(LAPACK REQUIRED)
  find_library(LAPACKE_LIB NAMES lapacke REQUIRED)
  set(LAPACK_LIBRARIES ${LAPACKE_LIB} ${LAPACK_LIBRARIES} ${BLAS_LIBRARIES})
  find_path(LAPACKE_INCLUDE_DIRS NAMES lapacke.h HINTS ${LAPACK_LIBRARIES})
  if(LAPACK_FOUND)
    message(STATUS "Lapacke libraries: ${LAPACK_LIBRARIES}")
    message(STATUS "Lapacke include dirs: ${LAPACKE_INCLUDE_DIRS}")
  else()
    message(FATAL_ERROR "Error in Lapacke detection ! lapacke not found !") 
  endif()

elseif(MSVC)
  set(OPENBLAS_ROOT_DIR $ENV{OPENBLAS_ROOT_DIR} CACHE PATH "Path to the OpenBLAS.")
  if(OPENBLAS_ROOT_DIR)
    list(APPEND CMAKE_PREFIX_PATH "${OPENBLAS_ROOT_DIR}")
    find_package(OpenBLAS REQUIRED)
  else()
    message(FATAL "Could not find OpenBLAS")
  endif()
  set(LAPACK_ROOT_DIR $ENV{LAPACK_ROOT_DIR} CACHE PATH "Path to the LAPACKE")
  if(LAPACK_ROOT_DIR)
    set(LAPACK_ROOT_DIR $ENV{LAPACK_ROOT_DIR} CACHE PATH "Path to the LAPACK.")
    list(APPEND CMAKE_PREFIX_PATH "${LAPACK_ROOT_DIR}")
  endif()
  find_package(LAPACK REQUIRED)
  find_library(LAPACKE_LIB NAMES openblas REQUIRED)
  set(LAPACK_LIBRARIES ${LAPACKE_LIB} ${LAPACK_LIBRARIES} ${BLAS_LIBRARIES})
  find_path(LAPACKE_INCLUDE_DIRS NAMES lapacke.h HINTS ${LAPACK_LIBRARIES})
  if(LAPACK_FOUND)
    message(STATUS "Lapacke libraries: ${LAPACK_LIBRARIES}")
    message(STATUS "Lapacke include dirs: ${LAPACKE_INCLUDE_DIRS}")
  else()
    message(FATAL_ERROR "Error in Lapacke detection ! lapacke not found !") 
  endif()

else()
  message(FATAL "Not implemented")
endif()


ADD_DEFINITIONS(${HDF5_DEFINITIONS} ${MEDFILE_DEFINITIONS})

IF (NOT DEFINED MSVC)
  ADD_DEFINITIONS(-Wsign-compare -Wconversion)
ENDIF()

IF(MEDCOUPLING_ENABLE_PYTHON)
  ADD_SUBDIRECTORY(Swig)
ENDIF(MEDCOUPLING_ENABLE_PYTHON)

IF(MEDCOUPLING_BUILD_TESTS)
  ADD_SUBDIRECTORY(Test)
ENDIF(MEDCOUPLING_BUILD_TESTS)

INCLUDE_DIRECTORIES(
  ${MEDFILE_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS}
  ${LAPACKE_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/../MEDCoupling
  ${CMAKE_CURRENT_SOURCE_DIR}/../INTERP_KERNEL
  ${CMAKE_CURRENT_SOURCE_DIR}/../INTERP_KERNEL/Bases
)

SET(shaperecogn_SOURCES
  MathOps.cxx
  Nodes.cxx
  NodesBuilder.cxx
  Areas.cxx
  AreasBuilder.cxx
  ShapeRecognMeshBuilder.cxx
  ShapeRecognMesh.cxx
  PrimitiveType.cxx
)

ADD_LIBRARY(shaperecogn ${shaperecogn_SOURCES})
SET_TARGET_PROPERTIES(shaperecogn PROPERTIES COMPILE_FLAGS "")
TARGET_LINK_LIBRARIES(shaperecogn medcouplingcpp ${LAPACK_LIBRARIES})
INSTALL(TARGETS shaperecogn EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${MEDCOUPLING_INSTALL_LIBS})

FILE(GLOB shaperecogn_HEADERS_HXX "${CMAKE_CURRENT_SOURCE_DIR}/*.hxx")

# To allow usage as SWIG dependencies:
SET(shaperecogn_HEADERS_HXX PARENT_SCOPE)

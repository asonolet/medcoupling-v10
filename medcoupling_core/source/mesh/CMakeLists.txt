# Copyright (C) 2012-2024  CEA, EDF
#
# This library is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2.1 of the License, or (at your option)
# any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License along
# with this library; if not, write to the Free Software Foundation, Inc., 59
# Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email :
# webmaster.salome@opencascade.com
#
# Author : Anthony Geay (CEA/DEN)

# KILLER WARNINGS TO HELP INT64 MANAGEMENT ADD_DEFINITIONS(-Wextra -Wsynth
# -Wno-unused-parameter -pedantic -Wall -Wshadow  -Wpointer-arith -Wcast-qual
# -Wcast-align -Wwrite-strings -Wctor-dtor-privacy -Wnon-virtual-dtor
# -Woverloaded-virtual -Wreturn-type  -Wno-endif-labels  -Wsign-compare
# -Wmissing-format-attribute -Wno-multichar -Wno-deprecated-declarations
# -Wpacked -Wredundant-decls -Wlong-long -Wdisabled-optimization
# -Wunknown-pragmas -Wundef  -Wreorder)

set(MEDCOUPLING_SOURCES
    MEDCouplingUMesh.cxx
    MEDCouplingUMesh_internal.cxx
    MEDCouplingUMesh_intersection.cxx
    MEDCoupling1GTUMesh.cxx
    MEDCouplingCMesh.cxx
    MEDCouplingIMesh.cxx
    MEDCouplingCurveLinearMesh.cxx
    MEDCouplingStructuredMesh.cxx
    MEDCouplingPointSet.cxx
    MEDCouplingMappedExtrudedMesh.cxx
    MEDCouplingMesh.cxx)

add_library(mesh OBJECT ${MEDCOUPLING_SOURCES})
target_compile_options(mesh PRIVATE -fPIC)

if(NOT DEFINED MSVC)
  target_compile_options(mesh PRIVATE -Wsign-compare -Wconversion)
endif()

target_include_directories(
  mesh
  PUBLIC
  "\$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
  "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/interp_kernel>"
  "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/medcoupling_core>"
)

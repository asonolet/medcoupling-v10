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

# KILLER WARNINGS TO HELP INT64 MANAGEMENT ADD_DEFINITIONS(-Wextra -Wsynth
# -Wno-unused-parameter -pedantic -Wall -Wshadow  -Wpointer-arith -Wcast-qual
# -Wcast-align -Wwrite-strings -Wctor-dtor-privacy -Wnon-virtual-dtor
# -Woverloaded-virtual -Wreturn-type  -Wno-endif-labels  -Wsign-compare
# -Wmissing-format-attribute -Wno-multichar -Wno-deprecated-declarations
# -Wpacked -Wredundant-decls -Wlong-long -Wdisabled-optimization
# -Wunknown-pragmas -Wundef  -Wreorder)

set(CORE_INTERPKERNEL_SOURCES
  interp_kernel/TransformedTriangle.cxx
  interp_kernel/TransformedTriangleIntersect.cxx
  interp_kernel/TransformedTriangleMath.cxx
  interp_kernel/BoundingBox.cxx
  interp_kernel/TranslationRotationMatrix.cxx
  interp_kernel/TetraAffineTransform.cxx
  interp_kernel/CellModel.cxx
  interp_kernel/DiameterCalculator.cxx
  interp_kernel/OrientationInverter.cxx
  interp_kernel/UnitTetraIntersectionBary.cxx
  interp_kernel/InterpolationOptions.cxx
  interp_kernel/BoxSplittingOptions.cxx
  interp_kernel/DirectedBoundingBox.cxx
  interp_kernel/Interpolation2DCurve.cxx
  interp_kernel/Interpolation3DSurf.cxx
  interp_kernel/Interpolation3D.cxx
  interp_kernel/Interpolation2D3D.cxx
  interp_kernel/Interpolation3D1D.cxx
  interp_kernel/Interpolation1D0D.cxx
  interp_kernel/MeshElement.cxx
  interp_kernel/InterpKernelMeshQuality.cxx
  interp_kernel/InterpKernelCellSimplify.cxx
  interp_kernel/InterpKernelMatrixTools.cxx
  interp_kernel/VolSurfUser.cxx
  interp_kernel/SplitterTetra.cxx
  interp_kernel/Bases/InterpKernelException.cxx
  interp_kernel/Geometric2D/InterpKernelGeo2DAbstractEdge.cxx
  interp_kernel/Geometric2D/InterpKernelGeo2DBounds.cxx
  interp_kernel/Geometric2D/InterpKernelGeo2DPrecision.cxx
  interp_kernel/Geometric2D/InterpKernelGeo2DComposedEdge.cxx
  interp_kernel/Geometric2D/InterpKernelGeo2DEdgeArcCircle.cxx
  interp_kernel/Geometric2D/InterpKernelGeo2DEdge.cxx
  interp_kernel/Geometric2D/InterpKernelGeo2DEdgeInfLin.cxx
  interp_kernel/Geometric2D/InterpKernelGeo2DEdgeLin.cxx
  interp_kernel/Geometric2D/InterpKernelGeo2DElementaryEdge.cxx
  interp_kernel/Geometric2D/InterpKernelGeo2DNode.cxx
  interp_kernel/Geometric2D/InterpKernelGeo2DQuadraticPolygon.cxx
  interp_kernel/ExprEval/InterpKernelExprParser.cxx
  interp_kernel/ExprEval/InterpKernelFunction.cxx
  interp_kernel/ExprEval/InterpKernelUnit.cxx
  interp_kernel/ExprEval/InterpKernelValue.cxx
  interp_kernel/ExprEval/InterpKernelAsmX86.cxx
  interp_kernel/GaussPoints/InterpKernelGaussCoords.cxx
  interp_kernel/LinearAlgebra/InterpKernelDenseMatrix.cxx
  interp_kernel/LinearAlgebra/InterpKernelLUDecomp.cxx
  interp_kernel/LinearAlgebra/InterpKernelQRDecomp.cxx)

set(CORE_ARRAY_SOURCES
  array/MEDCouplingMap.cxx
  array/MEDCouplingMatrix.cxx
  array/MEDCouplingMemArray.cxx
  array/MEDCouplingMemArrayChar.cxx
  array/MEDCouplingMemArrayFloat.cxx
  array/MEDCouplingPartDefinition.cxx
  array/MEDCouplingRefCountObject.cxx
  array/MEDCouplingSkyLineArray.cxx
  array/MEDCouplingTimeLabel.cxx
  array/MEDCouplingTraits.cxx)

set(CORE_MESH_SOURCES
  mesh/MEDCouplingUMesh.cxx
  mesh/MEDCouplingUMesh_internal.cxx
  mesh/MEDCouplingUMesh_intersection.cxx
  mesh/MEDCoupling1GTUMesh.cxx
  mesh/MEDCouplingCMesh.cxx
  mesh/MEDCouplingIMesh.cxx
  mesh/MEDCouplingCurveLinearMesh.cxx
  mesh/MEDCouplingStructuredMesh.cxx
  mesh/MEDCouplingPointSet.cxx
  mesh/MEDCouplingMappedExtrudedMesh.cxx
  mesh/MEDCouplingMesh.cxx)

set(CORE_FIELD_SOURCES
  field/MEDCouplingField.cxx
  field/MEDCouplingFieldFloat.cxx
  field/MEDCouplingFieldDouble.cxx
  field/MEDCouplingFieldInt32.cxx
  field/MEDCouplingFieldInt64.cxx
  field/MEDCouplingTimeDiscretization.cxx
  field/MEDCouplingFieldDiscretization.cxx
  field/MEDCouplingFieldDiscretizationOnNodesFE.cxx
  field/MEDCouplingFieldTemplate.cxx
  field/MEDCouplingGaussLocalization.cxx
  field/MEDCouplingNatureOfField.cxx
  field/MEDCouplingMultiFields.cxx
  field/MEDCouplingDefinitionTime.cxx
  field/MEDCouplingFieldOverTime.cxx
  field/MEDCouplingVoronoi.cxx)

set(CORE_AMR_SOURCES
  amr/MEDCouplingCartesianAMRMesh.cxx
  amr/MEDCouplingAMRAttribute.cxx)


add_library(medcouplingcpp
  ${CORE_INTERPKERNEL_SOURCES}
  ${CORE_ARRAY_SOURCES}
  ${CORE_MESH_SOURCES}
  ${CORE_FIELD_SOURCES}
  ${CORE_AMR_SOURCES})
add_library(MEDCoupling::medcoupling ALIAS medcouplingcpp)

set_target_properties(
  medcouplingcpp PROPERTIES
  INSTALL_RPATH ${MEDCOUPLING_INSTALL_LIBS_DEPS_RPATH}
  OUTPUT_NAME "medcoupling")

if(MEDCOUPLING_USE_64BIT_IDS)
  target_compile_definitions(medcouplingcpp PUBLIC "MEDCOULPING_USE_64BIT_IDS")
endif()
target_compile_options(medcouplingcpp PRIVATE -fsigned-char)
target_compile_options(medcouplingcpp PRIVATE -fPIC)

if(NOT DEFINED MSVC)
  target_compile_options(medcouplingcpp PRIVATE -Wsign-compare -Wconversion)
endif()

# if(WIN32)
#   target_link_options(medcouplingcpp PRIVATE "-Wl,--export-all-symbols")
# endif()

# target_link_libraries(medcouplingcpp PUBLIC $<TARGET_OBJECTS:interpkernel> $<TARGET_OBJECTS:array> $<TARGET_OBJECTS:mesh> $<TARGET_OBJECTS:field> $<TARGET_OBJECTS:amr>)

target_include_directories(medcouplingcpp
  PUBLIC
  "\$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
  "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/interp_kernel>"
  "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/array>"
  "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/mesh>"
  "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/amr>"
  "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/field>"
  # "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/polymesh>"
  "\$<INSTALL_INTERFACE:include>"
  "\$<INSTALL_INTERFACE:include/interp_kernel>"
  "\$<INSTALL_INTERFACE:include/array>"
  "\$<INSTALL_INTERFACE:include/mesh>"
  "\$<INSTALL_INTERFACE:include/amr>"
  "\$<INSTALL_INTERFACE:include/field>"
)

# TODO
# set_target_properties(medcouplingcpp PROPERTY PUBLIC_HEADERS ...)

install(
  TARGETS medcouplingcpp
  EXPORT ${CMAKE_PROJECT_NAME}TargetGroup
  DESTINATION ${MEDCOUPLING_INSTALL_LIBS}
  INCLUDES DESTINATION ${MEDCOUPLING_INSTALL_HEADERS})

# TODO: separate medfile from med dependancy and compile it into the main lib
# medfile_api needs to be splitten in medreader/medwriter (will go into
# medcoupling_io) and polymesh => which is not dependant on Medfile
if(NOT MEDCOUPLING_MICROMED)
  add_subdirectory(polymesh)
endif()

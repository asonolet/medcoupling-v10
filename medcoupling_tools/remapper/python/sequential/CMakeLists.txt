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

###################
# Remapper target #
###################

add_compile_definitions("WITH_NUMPY" "WITH_SCIPY")
set_source_files_properties(MEDCouplingRemapper.i PROPERTIES CPLUSPLUS ON)
if(WIN32)
  set_property(SOURCE MEDCouplingRemapper.i PROPERTY COMPILE_DEFINITIONS WIN32)
endif()

swig_add_library(
  MEDCouplingRemapper
  LANGUAGE python
  OUTPUT_DIR .
  SOURCES MEDCouplingRemapper.i)

target_include_directories(
  ${SWIG_MODULE_MEDCouplingRemapper_REAL_NAME}
  PRIVATE
  "\$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/medcoupling_core/python/medcoupling_core>"
)

set_property(TARGET ${SWIG_MODULE_MEDCouplingRemapper_REAL_NAME} PROPERTY SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE)
set_property(TARGET ${SWIG_MODULE_MEDCouplingRemapper_REAL_NAME} PROPERTY INSTALL_RPATH ${MEDCOUPLING_INSTALL_PYTHON_LIBS_DEPS_RPATH})

target_link_libraries(${SWIG_MODULE_MEDCouplingRemapper_REAL_NAME}
  PUBLIC medcouplingremapper
  PRIVATE Python::Module Python::NumPy)

if(WIN32)
  set_target_properties(${SWIG_MODULE_MEDCouplingRemapper_REAL_NAME}
    PROPERTIES DEBUG_OUTPUT_NAME _MEDCouplingRemapper_d)
  set_target_properties(
    ${SWIG_MODULE_pymedcoupling_REAL_NAME}
    PROPERTIES DEBUG_OUTPUT_NAME _${MEDCouling_target_name}_d)
endif()

################
# Installation #
################

install(TARGETS
  ${SWIG_MODULE_MEDCouplingRemapper_REAL_NAME}
  # RUNTIME_DEPENDENCIES DIRECTORIES ${MEDCOUPLING_INSTALL_LIBS}
  DESTINATION ${MEDCOUPLING_INSTALL_PYTHON})

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/MEDCouplingRemapper.py
  DESTINATION ${MEDCOUPLING_INSTALL_PYTHON})

set(BASE_TESTS
  MEDCouplingRemapperTest.py
  RemapperUserGuideExamplesTest.py
)

# This test needs medcoupling install/lib/pythonX.Y/site-package to be in path
# to run
foreach(test ${BASE_TESTS})
  get_filename_component(testname ${test} NAME_WE)
  add_test(
    NAME py${testname}
    COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${test})
endforeach()

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
if(NOT DEFINED MSVC)
  add_definitions(-Wsign-compare -Wconversion)
endif()

set_source_files_properties(
  medcoupling_core.i PROPERTIES
  CPLUSPLUS ON
  COMPILE_DEFINITIONS MEDCOUPLING_SWIG4_COMPAT
)


if(WIN32)
  set_property(SOURCE medcoupling_core.i PROPERTY COMPILE_DEFINITIONS WIN32)
endif()

# set(MC_pyTestFiles ${ALL_TESTS} MEDCouplingDataForTest.py)

# _ABR_ Ensure dependency mechanism on all SWIG files and headers
# set(SWIG_MODULE_MEDCoupling_EXTRA_DEPS
#     ${MEDCoupling_SWIG_DPYS_FILES} ${medcoupling_HEADERS_HXX}
#     ${medcoupling_HEADERS_TXX} ${interpkernel_HEADERS_HXX}
#     ${interpkernel_HEADERS_TXX})

# SWIG must run after the doc if we want to have the docstrings extracted from
# Doxygen into the Python module:
# if(MEDCOUPLING_BUILD_DOC)
#   list(APPEND SWIG_MODULE_MEDCoupling_EXTRA_FLAGS -DWITH_DOCSTRINGS)
#   list(APPEND SWIG_MODULE_MEDCoupling_EXTRA_DEPS
#        ${PROJECT_BINARY_DIR}/doc/MEDCoupling_doc.i swig_ready)
# endif()

swig_add_library(
  pymedcoupling_core
  LANGUAGE python
  SOURCES medcoupling_core.i)

target_include_directories(
  ${SWIG_MODULE_pymedcoupling_core_REAL_NAME}
  PRIVATE
  "\$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

set_target_properties(
  ${SWIG_MODULE_pymedcoupling_core_REAL_NAME} PROPERTIES
  INSTALL_RPATH ${MEDCOUPLING_INSTALL_PYTHON_LIBS_DEPS_RPATH}
  SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE)

target_link_libraries(${SWIG_MODULE_pymedcoupling_core_REAL_NAME}
  PUBLIC medcouplingcpp
  PRIVATE Python::Module Python::NumPy)

if(WIN32)
  target_compile_options(${SWIG_MODULE_pymedcoupling_core_REAL_NAME} PRIVATE /bigobj)
endif()

################
# Installation #
################

install(TARGETS ${SWIG_MODULE_pymedcoupling_core_REAL_NAME}
  # RUNTIME_DEPENDENCIES DIRECTORIES ${MEDCOUPLING_INSTALL_LIBS}
  DESTINATION ${MEDCOUPLING_INSTALL_PYTHON})

install(
  TARGETS ${SWIG_MODULE_pymedcoupling_core_REAL_NAME}
  DESTINATION ${MEDCOUPLING_INSTALL_PYTHON})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/medcoupling_core.py DESTINATION ${MEDCOUPLING_INSTALL_PYTHON})

install(FILES vtk2medcoupling.py geom2medcoupling.py
  DESTINATION ${MEDCOUPLING_INSTALL_PYTHON})

###########
# TESTING #
###########

set(BASE_TESTS
  MEDCouplingBasicsTest1.py
  MEDCouplingBasicsTest2.py
  MEDCouplingBasicsTest3.py
  MEDCouplingBasicsTest4.py
  MEDCouplingBasicsTest5.py
  MEDCouplingBasicsTest6.py
  MEDCouplingBasicsTest7.py
  MEDCouplingIntersectTest.py
  MEDCouplingExamplesTest.py
  UsersGuideExamplesTest.py
)

# if numpy is used
set(NUMPY_TESTS
  MEDCouplingNumPyTest.py
  MEDCouplingPickleTest.py
  UsersGuideExamplesTest_numpy.py
)

configure_file(MEDCouplingDataForTest.py MEDCouplingDataForTest.py)

if(NUMPY_FOUND)
  set(ALL_TESTS ${BASE_TESTS} ${NUMPY_TESTS})
else()
  set(ALL_TESTS ${BASE_TESTS})
endif()

if(WIN32)
  set(DLL_PATHS "$<SHELL_PATH:$<TARGET_RUNTIME_DLL_DIRS:${SWIG_MODULE_pymedcoupling_core_REAL_NAME}>>")
  set(PATH_TO_RUNDIR "${CMAKE_CURRENT_BINARY_DIR}$<SEMICOLON>${DLL_PATHS}$<SEMICOLON>$ENV{PATH}$<SEMICOLON>")
  add_custom_target(genexdebug3 COMMAND ${CMAKE_COMMAND} -E echo "${SWIG_MODULE_pymedcoupling_core_REAL_NAME} dll paths: ${DLL_PATHS}")
    # get_target_property(PATH_TO_RUNDIR ${SWIG_MODULE_pymedcoupling_core_REAL_NAME} RUNTIME_OUTPUT_DIRECTORY)
else()
  set(PATH_TO_RUNDIR ${CMAKE_CURRENT_BINARY_DIR})
endif()

foreach(test ${ALL_TESTS})
  get_filename_component(testname ${test} NAME_WE)
  add_test(
    NAME py${testname}
    COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${test})
  set_tests_properties(py${testname} PROPERTIES
      ENVIRONMENT "PYTHONPATH=${PATH_TO_RUNDIR}$<SEMICOLON>$ENV{PYTHONPATH}")

  if(WIN32)
    # NOTE ici on fait l'hypothèse que sous windows tout les binaires runtime
    # sont dans le même dossier
    set_tests_properties(
      py${testname} PROPERTIES
      ENVIRONMENT "PATH=${PATH_TO_RUNDIR}$<SEMICOLON>$ENV{PATH}"
    )
  endif()
endforeach()

# # Application tests
# 
# set(TEST_INSTALL_DIRECTORY ${MEDCOUPLING_INSTALL_TESTS}/MEDCoupling_Swig)
# install(FILES ${MC_pyTestFiles} DESTINATION ${TEST_INSTALL_DIRECTORY})
# 
# install(
#   FILES CTestTestfileInstall.cmake
#   DESTINATION ${TEST_INSTALL_DIRECTORY}
#   RENAME CTestTestfile.cmake)
# install(FILES tests.set DESTINATION ${TEST_INSTALL_DIRECTORY})

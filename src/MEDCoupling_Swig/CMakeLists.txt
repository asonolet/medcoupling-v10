# Copyright (C) 2012-2024  CEA, EDF
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
IF(MEDCOUPLING_BUILD_PY_TESTS)
    INCLUDE(tests.set)
ENDIF()

INCLUDE(${SWIG_USE_FILE})

ADD_DEFINITIONS(${PYTHON_DEFINITIONS} ${NUMPY_DEFINITIONS} ${SCIPY_DEFINITIONS})

IF (NOT DEFINED MSVC)
  ADD_DEFINITIONS(-Wsign-compare -Wconversion)
ENDIF()

SET_SOURCE_FILES_PROPERTIES(MEDCoupling.i PROPERTIES CPLUSPLUS ON)
IF ("${PYTHON_VERSION_MAJOR}" STREQUAL "3")
  SET_SOURCE_FILES_PROPERTIES(MEDCoupling.i PROPERTIES SWIG_FLAGS "-py3")
ELSE()
  SET_SOURCE_FILES_PROPERTIES(MEDCoupling.i PROPERTIES SWIG_DEFINITIONS "-shadow")
ENDIF()
SET(SWIG_MODULE_MEDCoupling_EXTRA_FLAGS "${NUMPY_DEFINITIONS};${SCIPY_DEFINITIONS}")
IF(MEDCOUPLING_USE_64BIT_IDS)
  STRING(APPEND SWIG_MODULE_MEDCoupling_EXTRA_FLAGS ";-DMEDCOUPLING_USE_64BIT_IDS")
ENDIF()

SET (MEDCoupling_SWIG_DPYS_FILES
    MEDCouplingCommon.i
    MEDCouplingMemArray.i
    DataArrayInt.i
    MEDCouplingFieldDiscretization.i
    MEDCouplingFinalize.i
    MEDCouplingTypemaps.i
    ../ICoCo/Swig/ICoCoMEDField.i
)

SET (MC_pyTestFiles
    ${ALL_TESTS}
    MEDCouplingDataForTest.py
)

SET (MC_Swig_interf
    MEDCoupling.i
    MEDCouplingCommon.i
    MEDCouplingRemapperCommon.i
    MEDCouplingRefCountObject.i
    MEDCouplingMemArray.i
    DataArrayInt.i
    MEDCouplingFieldDiscretization.i
    MEDCouplingTimeDiscretization.i
    MEDCouplingFinalize.i
    MEDCouplingRemapper.i
    MEDCouplingTypemaps.i
    MEDCouplingDataArrayTypemaps.i
    MEDCouplingDataArrayTraits.hxx
    ../ICoCo/Swig/ICoCoMEDField.i
)

INCLUDE_DIRECTORIES(
  ${PYTHON_INCLUDE_DIRS}
  ${NUMPY_INCLUDE_DIR}
  ${PTHREAD_INCLUDE_DIR} # pthread dependency due to python2.7 library
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../MEDCoupling
  ${CMAKE_CURRENT_SOURCE_DIR}/../INTERP_KERNEL
  ${CMAKE_CURRENT_SOURCE_DIR}/../INTERP_KERNEL/Bases
  ${CMAKE_CURRENT_SOURCE_DIR}/../INTERP_KERNEL/Geometric2D
  ${CMAKE_CURRENT_SOURCE_DIR}/../INTERP_KERNEL/ExprEval
  ${CMAKE_CURRENT_SOURCE_DIR}/../INTERP_KERNEL/GaussPoints
  ${CMAKE_CURRENT_SOURCE_DIR}/../ICoCo
  ${CMAKE_CURRENT_SOURCE_DIR}/../ICoCo/Swig
  ${PROJECT_BINARY_DIR}/doc
  )

# _ABR_ Ensure dependency mechanism on all SWIG files and headers
SET (SWIG_MODULE_MEDCoupling_EXTRA_DEPS ${MEDCoupling_SWIG_DPYS_FILES}
    ${medcoupling_HEADERS_HXX} ${medcoupling_HEADERS_TXX}
    ${interpkernel_HEADERS_HXX} ${interpkernel_HEADERS_TXX})

# SWIG must run after the doc if we want to have the docstrings extracted from Doxygen
# into the Python module:
IF(MEDCOUPLING_BUILD_DOC)
    LIST(APPEND SWIG_MODULE_MEDCoupling_EXTRA_FLAGS -DWITH_DOCSTRINGS)
    LIST(APPEND SWIG_MODULE_MEDCoupling_EXTRA_DEPS
        ${PROJECT_BINARY_DIR}/doc/MEDCoupling_doc.i
        swig_ready)
ENDIF()

IF(WIN32)
  # Under Windows MEDCoupling conflicts wiht medcoupling from PyWrapping
  SET(MEDCouling_target_name MEDCouplingCompat)
  SET_PROPERTY(SOURCE MEDCoupling.i PROPERTY SWIG_MODULE_NAME MEDCouplingCompat)
  SET_PROPERTY(SOURCE MEDCoupling.i PROPERTY COMPILE_DEFINITIONS WIN32)
  SET(SWIG_MODULE_MEDCouplingCompat_EXTRA_FLAGS "${NUMPY_DEFINITIONS};${SCIPY_DEFINITIONS}")
  IF(MEDCOUPLING_USE_64BIT_IDS)
    STRING(APPEND SWIG_MODULE_MEDCouplingCompat_EXTRA_FLAGS ";-DMEDCOUPLING_USE_64BIT_IDS")
  ENDIF(MEDCOUPLING_USE_64BIT_IDS)
ELSE()
  SET(MEDCouling_target_name MEDCoupling)
ENDIF()

IF(${CMAKE_VERSION} VERSION_LESS "3.8.0") 
  SWIG_ADD_MODULE(${MEDCouling_target_name} python MEDCoupling.i)  
ELSE()
  SWIG_ADD_LIBRARY(${MEDCouling_target_name} LANGUAGE python SOURCES MEDCoupling.i)
ENDIF()

IF(WIN32 OR CYGWIN)
  SWIG_LINK_LIBRARIES(${MEDCouling_target_name} ${PYTHON_LIBRARIES} ${PLATFORM_LIBS} medcouplingcpp)
ELSE(WIN32 OR CYGWIN)
  # ${PYTHON_LIBRARIES} not needed see https://www.python.org/dev/peps/pep-0513/#libpythonx-y-so-1
  SWIG_LINK_LIBRARIES(${MEDCouling_target_name} ${PLATFORM_LIBS} medcouplingcpp)
ENDIF(WIN32 OR CYGWIN)
# ${PYTHON_LIBRARIES} not needed
SWIG_CHECK_GENERATION(${MEDCouling_target_name})
SET_SOURCE_FILES_PROPERTIES(MEDCouplingRemapper.i PROPERTIES CPLUSPLUS ON)
IF(WIN32)
  SET_PROPERTY(SOURCE MEDCouplingRemapper.i PROPERTY COMPILE_DEFINITIONS WIN32)
ENDIF()
IF ("${PYTHON_VERSION_MAJOR}" STREQUAL "3")
  SET_SOURCE_FILES_PROPERTIES(MEDCouplingRemapper.i PROPERTIES SWIG_FLAGS "-py3")
ELSE()
  SET_SOURCE_FILES_PROPERTIES(MEDCouplingRemapper.i PROPERTIES SWIG_DEFINITIONS "-shadow")
ENDIF()
SET(SWIG_MODULE_MEDCouplingRemapper_EXTRA_FLAGS "${NUMPY_DEFINITIONS};${SCIPY_DEFINITIONS}")
IF(MEDCOUPLING_USE_64BIT_IDS)
  STRING(APPEND SWIG_MODULE_MEDCouplingRemapper_EXTRA_FLAGS ";-DMEDCOUPLING_USE_64BIT_IDS")
ENDIF(MEDCOUPLING_USE_64BIT_IDS)

# _ABR_ Ensure dependency mechanism on all SWIG files and headers
SET (SWIG_MODULE_MEDCouplingRemapper_EXTRA_DEPS ${MEDCoupling_SWIG_DPYS_FILES}
    ${medcoupling_HEADERS_HXX} ${medcoupling_HEADERS_TXX}
    ${interpkernel_HEADERS_HXX} ${interpkernel_HEADERS_TXX})


IF(${CMAKE_VERSION} VERSION_LESS "3.8.0") 
  SWIG_ADD_MODULE(MEDCouplingRemapper python MEDCouplingRemapper.i)  
ELSE()
  SWIG_ADD_LIBRARY(MEDCouplingRemapper LANGUAGE python SOURCES MEDCouplingRemapper.i)
ENDIF()

IF(WIN32 OR CYGWIN)
  SWIG_LINK_LIBRARIES(MEDCouplingRemapper ${PYTHON_LIBRARIES} medcouplingremapper)
ELSE(WIN32 OR CYGWIN)
  # ${PYTHON_LIBRARIES} not needed see https://www.python.org/dev/peps/pep-0513/#libpythonx-y-so-1
  SWIG_LINK_LIBRARIES(MEDCouplingRemapper medcouplingremapper)
ENDIF(WIN32 OR CYGWIN)

IF(WIN32)
  SET_TARGET_PROPERTIES(_MEDCouplingRemapper PROPERTIES DEBUG_OUTPUT_NAME _MEDCouplingRemapper_d)
  SET_TARGET_PROPERTIES(_${MEDCouling_target_name} PROPERTIES DEBUG_OUTPUT_NAME _${MEDCouling_target_name}_d)
ENDIF(WIN32)
INSTALL(TARGETS ${SWIG_MODULE_${MEDCouling_target_name}_REAL_NAME} ${SWIG_MODULE_MEDCouplingRemapper_REAL_NAME} DESTINATION ${MEDCOUPLING_INSTALL_PYTHON})

SET(PYFILES_TO_INSTALL ${CMAKE_CURRENT_BINARY_DIR}/MEDCouplingRemapper.py)
IF(WIN32)
  SET(PYFILES_TO_INSTALL ${PYFILES_TO_INSTALL} ${CMAKE_CURRENT_BINARY_DIR}/MEDCouplingCompat.py)
ELSE()
  SET(PYFILES_TO_INSTALL ${PYFILES_TO_INSTALL} ${CMAKE_CURRENT_BINARY_DIR}/MEDCoupling.py)
ENDIF()
INSTALL(FILES vtk2medcoupling.py geom2medcoupling.py DESTINATION ${MEDCOUPLING_INSTALL_PYTHON})

SALOME_INSTALL_SCRIPTS("${PYFILES_TO_INSTALL}" ${MEDCOUPLING_INSTALL_PYTHON} EXTRA_DPYS "${SWIG_MODULE_${MEDCouling_target_name}_REAL_NAME};${SWIG_MODULE_MEDCouplingRemapper_REAL_NAME}")

INSTALL(FILES ${MC_Swig_interf} DESTINATION ${MEDCOUPLING_INSTALL_HEADERS})
INSTALL(FILES ${MC_pyTestFiles} DESTINATION ${MEDCOUPLING_INSTALL_SCRIPT_PYTHON})

SALOME_ACCUMULATE_ENVIRONMENT(PYTHONPATH NOCHECK ${CMAKE_CURRENT_BINARY_DIR}/../PyWrapping)
SALOME_GENERATE_TESTS_ENVIRONMENT(tests_env)

FOREACH(test ${BASE_TESTS})
  GET_FILENAME_COMPONENT(testname ${test} NAME_WE)
  ADD_TEST(NAME ${testname}
           COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${test})
  SET_TESTS_PROPERTIES(${testname} PROPERTIES ENVIRONMENT "${tests_env}")
ENDFOREACH()

IF(NUMPY_FOUND)
  FOREACH(test ${NUMPY_TESTS})
    GET_FILENAME_COMPONENT(testname ${test} NAME_WE)
    ADD_TEST(NAME ${testname}
             COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${test})
    SET_TESTS_PROPERTIES(${testname} PROPERTIES ENVIRONMENT "${tests_env}")
  ENDFOREACH()
ENDIF(NUMPY_FOUND)

# Application tests

SET(TEST_INSTALL_DIRECTORY ${MEDCOUPLING_INSTALL_TESTS}/MEDCoupling_Swig)
INSTALL(FILES ${MC_pyTestFiles} DESTINATION ${TEST_INSTALL_DIRECTORY})

INSTALL(FILES CTestTestfileInstall.cmake
        DESTINATION ${TEST_INSTALL_DIRECTORY}
        RENAME CTestTestfile.cmake)
INSTALL(FILES tests.set DESTINATION ${TEST_INSTALL_DIRECTORY})

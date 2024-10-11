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

set(TestMEDCoupling_SOURCES
  MEDCouplingBasicsTestInterp.cxx
  TestMEDCoupling.cxx
  MEDCouplingBasicsTest1.cxx
  MEDCouplingBasicsTest2.cxx
  MEDCouplingBasicsTest3.cxx
  MEDCouplingBasicsTest4.cxx
  MEDCouplingBasicsTest5.cxx)

set(TestMEDCouplingExamples_SOURCES MEDCouplingExamplesTest.cxx)

set(MEDCouplingTestUtils_SOURCES MEDCouplingBasicsTest0.cxx)

add_library(MEDCouplingTestUtils ${MEDCouplingTestUtils_SOURCES})
target_include_directories(MEDCouplingTestUtils PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../interp_kernel>
)

if(WIN32)
  set_property(TARGET MEDCouplingTestUtils PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

target_link_libraries(MEDCouplingTestUtils PRIVATE medcouplingcpp cppunit::cppunit)

add_executable(TestMEDCoupling ${TestMEDCoupling_SOURCES})

target_link_libraries(
  TestMEDCoupling
  PUBLIC MEDCouplingTestUtils medcouplingcpp cppunit::cppunit)

add_test(TestMEDCoupling TestMEDCoupling)

if(WIN32)
  # Ensure the required DLLs can be found at runtime
  set(DLL_PATH $<SHELL_PATH:$<TARGET_RUNTIME_DLL_DIRS:TestMEDCoupling>>$<SEMICOLON>)
  set_tests_properties(
    TestMEDCoupling PROPERTIES
    ENVIRONMENT "PATH=${DLL_PATH}$ENV{PATH}"
  )
  message(STATUS "DLL PATH set to " ${DLL_PATH})
endif()

add_executable(TestMEDCouplingExamples ${TestMEDCouplingExamples_SOURCES})
target_link_libraries(
  TestMEDCouplingExamples
  PUBLIC MEDCouplingTestUtils medcouplingcpp cppunit::cppunit)

add_test(TestMEDCouplingExamples TestMEDCouplingExamples)

if(WIN32)
  # Ensure the required DLLs can be found at runtime
  set(DLL_PATH $<SHELL_PATH:$<TARGET_RUNTIME_DLL_DIRS:TestMEDCouplingExamples>>$<SEMICOLON>)
  set_tests_properties(
    TestMEDCouplingExamples PROPERTIES
    ENVIRONMENT "PATH=${DLL_PATH}$ENV{PATH}"
  )
  message(STATUS "DLL PATH set to " ${DLL_PATH})
endif()

# set_tests_properties(TestMEDCouplingExamples PROPERTIES ENVIRONMENT "${tests_env}")

install(TARGETS TestMEDCoupling TestMEDCouplingExamples
  DESTINATION ${MEDCOUPLING_INSTALL_BINS})

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES UnitTestsResult)

# Application tests

# set(TEST_INSTALL_DIRECTORY ${MEDCOUPLING_INSTALL_TESTS}/MEDCoupling)
# install(
#   TARGETS TestMEDCoupling TestMEDCouplingExamples
#   DESTINATION ${TEST_INSTALL_DIRECTORY})

# install(
#   FILES CTestTestfileInstall.cmake
#   DESTINATION ${TEST_INSTALL_DIRECTORY}
#   RENAME CTestTestfile.cmake)

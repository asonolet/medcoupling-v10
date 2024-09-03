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

find_package(cppunit REQUIRED)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../interp_kernel)

set(TestMEDCouplingRemapper_SOURCES
  TestMEDCouplingRemapper.cxx
  MEDCouplingRemapperTest.cxx)

add_executable(TestMEDCouplingRemapper ${TestMEDCouplingRemapper_SOURCES})

target_link_libraries(TestMEDCouplingRemapper medcouplingremapper cppunit::cppunit)

add_test(TestMEDCouplingRemapper TestMEDCouplingRemapper)

# set_tests_properties(TestMEDCouplingRemapper PROPERTIES ENVIRONMENT "${tests_env}")

install(TARGETS TestMEDCouplingRemapper
  DESTINATION ${MEDCOUPLING_INSTALL_BINS})

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES UnitTestsResult)

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

set(MEDCOUPLINGREMAPPER_SOURCES MEDCouplingRemapper.cxx)
add_library(medcouplingremapper ${MEDCOUPLINGREMAPPER_SOURCES})
target_link_libraries(medcouplingremapper PUBLIC medcouplingcpp)

set_target_properties(
  medcouplingremapper PROPERTIES
  INSTALL_RPATH ${MEDCOUPLING_INSTALL_LIBS_DEPS_RPATH})

if(MEDCOUPLING_USE_64BIT_IDS)
  target_compile_definitions(medcouplingremapper PUBLIC "MEDCOULPING_USE_64BIT_IDS")
endif()

install(
  TARGETS medcouplingremapper
  EXPORT ${CMAKE_PROJECT_NAME}TargetGroup
  DESTINATION ${MEDCOUPLING_INSTALL_LIBS}
  INCLUDES DESTINATION ${MEDCOUPLING_INSTALL_HEADERS})

file(GLOB medcoupling_HEADERS "${PROJECT_SOURCE_DIR}/include/medcoupling_core/*")
install(FILES ${medcoupling_HEADERS} DESTINATION ${MEDCOUPLING_INSTALL_HEADERS})

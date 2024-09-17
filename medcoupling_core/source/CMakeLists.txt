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

add_subdirectory(interp_kernel)
add_subdirectory(array)
add_subdirectory(mesh)
add_subdirectory(field)
add_subdirectory(amr)


add_library(medcouplingcpp)
add_library(MEDCoupling::medcoupling ALIAS medcouplingcpp)

set_target_properties(
  medcouplingcpp PROPERTIES
  INSTALL_RPATH ${MEDCOUPLING_INSTALL_LIBS_DEPS_RPATH}
  OUTPUT_NAME "medcoupling")

if(MEDCOUPLING_USE_64BIT_IDS)
  target_compile_definitions(medcouplingcpp PUBLIC "MEDCOULPING_USE_64BIT_IDS")
endif()

target_link_libraries(medcouplingcpp PRIVATE interpkernel array mesh field amr)

target_include_directories(medcouplingcpp
  PUBLIC
  "\$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
  "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/interp_kernel>"
  "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/array>"
  "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/mesh>"
  "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/amr>"
  "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/field>"
  "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/polymesh>"
)

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

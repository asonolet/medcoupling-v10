# Copyright (C) 2015-2024  CEA, EDF
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
# Authors: Aymeric SONOLET

cmake_minimum_required(VERSION 3.24)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(
      FATAL_ERROR
      "In-source builds are not supported. "
      "Please read the BUILDING document before trying to build this project. "
      "You may need to delete 'CMakeCache.txt' and 'CMakeFiles/' first."
  )
endif()

project(
  medcoupling_core
  VERSION 10.0.0
  DESCRIPTION
    "medcoupling_core is a C++ library defining a data format for arrays, meshes, fields and some other usefull related things. It has no dependencies and will keep being that way."
  HOMEPAGE_URL "https://github.com/SalomePlatform/medcoupling"
  LANGUAGES CXX)

###########
# OPTIONS #
###########

option(MEDCOUPLING_ENABLE_PYTHON "Build PYTHON bindings." ON)
option(MEDCOUPLING_USE_64BIT_IDS "Size of IDs to refer cells and nodes." ON)
option(MEDCOUPLING_USE_MPI "Adds MPI capabilities to medcoupling_core" OFF)
option(MEDCOUPLING_MICROMED "Switch off med dependency" OFF)

###########
# VERSION #
###########

macro(tohexa num result)
  set(_hexa_map a b c d e f)
  if(${num} LESS 10)
    set(${result} "0${num}")
  else()
    math(EXPR _res "${num}-10" )
    list(GET _hexa_map ${_res} _out)
    set(${result} "0${_out}")
  endif()
endmacro()

tohexa(${${PROJECT_NAME}_VERSION_MAJOR} _major)
tohexa(${${PROJECT_NAME}_VERSION_MINOR} _minor)
tohexa(${${PROJECT_NAME}_VERSION_PATCH} _patch)
set(${PROJECT_NAME}_XVERSION "0x${_major}${_minor}${_patch}")

configure_file(include/MEDCoupling_version.h.in include/MEDCoupling_version.h)

#########
# FLAGS #
#########

add_library(medcoupling_core_flags INTERFACE)

if(MEDCOUPLING_USE_64BIT_IDS)
  target_compile_definitions(medcoupling_core_flags PUBLIC "MEDCOULPING_USE_64BIT_IDS")
endif()

if(NOT DEFINED MSVC)
  target_compile_options(medcoupling_core_flags PUBLIC -fsigned-char)
  target_compile_options(medcoupling_core_flags PUBLIC -Wsign-compare -Wconversion)
endif()

###########
# SOURCES #
###########

add_subdirectory(source)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()

if(MEDCOUPLING_ENABLE_PYTHON)
  add_subdirectory(python)
endif()

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

cmake_minimum_required(VERSION 3.24)

include(cmake/prelude.cmake)

project(
  MEDCoupling
  VERSION 10.0.0
  DESCRIPTION
    "MEDCoupling is a C++ library usefull for mesh manipulation and interpolation of fields."
  HOMEPAGE_URL "https://github.com/SalomePlatform/medcoupling"
  LANGUAGES CXX)

cmake_policy(SET CMP0091 NEW) # The policy for not installing in user space.
cmake_policy(SET CMP0144 NEW) # The policy for using PKGNM_ROOT for PkgNm module

#
# User options
# ============
option(MEDCOUPLING_MICROMED "Build MED without MED file dependency." ON)
option(MEDCOUPLING_ENABLE_PYTHON "Build PYTHON bindings." ON)
option(MEDCOUPLING_ENABLE_PARTITIONER "Build MEDPartitioner." OFF)
option(MEDCOUPLING_ENABLE_RENUMBER "Build Renumber." OFF)
option(MEDCOUPLING_ENABLE_SHAPERECOGN "Build Shape Recognition module." OFF)
option(
  MEDCOUPLING_WITH_FILE_EXAMPLES
  "Install examples of files containing meshes and fields of different formats."
  OFF)
option(
  MEDCOUPLING_USE_MPI
  "(Use MPI containers) - For MED this triggers the build of ParaMEDMEM."
  OFF)
option(MEDCOUPLING_BUILD_TESTS "Build MEDCoupling C++ tests." ON)
option(MEDCOUPLING_BUILD_PY_TESTS "Build MEDCoupling Python tests." OFF)
option(MEDCOUPLING_BUILD_DOC "Build MEDCoupling doc." OFF)
option(MEDCOUPLING_BUILD_STATIC "Build MEDCoupling library in static mode." OFF)
option(
  MEDCOUPLING_USE_64BIT_IDS
  "Size of IDs to refer cells and nodes. 32 bits when OFF, 64 bits when ON (default)."
  ON)

include(CMakeDependentOption)
cmake_dependent_option(
  MEDCOUPLING_PARTITIONER_METIS "Enable metis graph library in MEDPartitioner."
  ON "NOT MEDCOUPLING_USE_MPI;MEDCOUPLING_ENABLE_PARTITIONER" OFF)
cmake_dependent_option(
  MEDCOUPLING_PARTITIONER_SCOTCH
  "Enable scotch graph library in MEDPartitioner." ON
  "NOT MEDCOUPLING_USE_MPI;MEDCOUPLING_ENABLE_PARTITIONER" OFF)
cmake_dependent_option(
  MEDCOUPLING_PARTITIONER_PARMETIS
  "Enable parmetis graph library in MEDPartitioner." ON
  "MEDCOUPLING_ENABLE_PARTITIONER;MEDCOUPLING_USE_MPI" OFF)
cmake_dependent_option(
  MEDCOUPLING_PARTITIONER_PTSCOTCH
  "Enable ptscotch graph library in MEDPartitioner." ON
  "MEDCOUPLING_ENABLE_PARTITIONER;MEDCOUPLING_USE_MPI" OFF)

set(XDR_DEFAULT_OPTION OFF)
if(WIN32)
  set(XDR_DEFAULT_OPTION OFF)
endif()

cmake_dependent_option(
  MEDCOUPLING_MEDLOADER_USE_XDR "Enable use of XDR for SauvReader."
  ${XDR_DEFAULT_OPTION} "NOT MEDCOUPLING_MICROMED" OFF)
cmake_dependent_option(
  MEDCOUPLING_BUILD_FRENCH_DOC "Generate MEDCOUPLING French documentation" OFF
  "MEDCOUPLING_BUILD_DOC" OFF)

if(MEDCOUPLING_BUILD_STATIC)
  set(BUILD_SHARED_LIBS 0)
  set(CMAKE_POSITION_INDEPENDENT_CODE 1) # -fPIC option
else()
  set(BUILD_SHARED_LIBS 1)
endif()

if(MEDCOUPLING_ENABLE_PYTHON)
  find_package(
    Python
    COMPONENTS Interpreter Development.Module NumPy
    REQUIRED)

  find_package(
    SWIG
    COMPONENTS python
    REQUIRED)

  include(UseSWIG)

  list(APPEND CMAKE_SWIG_FLAGS "-DWITH_NUMPY")
  list(APPEND CMAKE_SWIG_FLAGS "-DWITH_SCIPY")

  if(MEDCOUPLING_USE_64BIT_IDS)
    list(APPEND CMAKE_SWIG_FLAGS "-DMEDCOUPLING_USE_64BIT_IDS")
  endif()
endif()

# Directories
#
# Directories have to be given after python prerequisites (to be able to use Python
# version string for example).
# ===========

if(NOT SKBUILD)
  set(MEDCOUPLING_INSTALL_BINS
    bin
    CACHE PATH "Install path: MEDCoupling binaries and scripts")
  set(MEDCOUPLING_INSTALL_LIBS
    lib
    CACHE PATH "Install path: MEDCoupling libs")
  if(MEDCOUPLING_ENABLE_PYTHON)
    set(MEDCOUPLING_INSTALL_PYTHON
      lib/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages
      CACHE PATH "Install path: MEDCoupling libs")
  endif()
  set(MEDCOUPLING_INSTALL_HEADERS
    include
    CACHE PATH "Install path: MEDCoupling headers")
  set(MEDCOUPLING_INSTALL_TESTS
    test
    CACHE PATH "Install path: MEDCoupling tests")
  set(MEDCOUPLING_INSTALL_CMAKE_LOCAL
    cmake_files
    CACHE PATH "Install path: local MEDCoupling CMake files")
  set(MEDCOUPLING_INSTALL_RES
    share/resources
    CACHE PATH "Install path: MEDCoupling resources")
  set(MEDCOUPLING_INSTALL_DOCS
    share/doc
    CACHE PATH "Install path: MEDCoupling documentation")

  # Origin 
  set(MEDCOUPLING_INSTALL_LIBS_DEPS_RPATH $ORIGIN/../${MEDCOUPLING_INSTALL_LIBS})
  set(MEDCOUPLING_INSTALL_PYTHON_LIBS_DEPS_RPATH $ORIGIN/../../../${MEDCOUPLING_INSTALL_LIBS})
else()
  set(MEDCOUPLING_INSTALL_BINS
    ${SKBUILD_SCRIPTS_DIR}
    CACHE PATH "Install path: MEDCoupling binaries and scripts")
  set(MEDCOUPLING_INSTALL_LIBS
    ${SKBUILD_PLATLIB_DIR}/medcoupling.libs
    CACHE PATH "Install path: MEDCoupling libs")
  set(MEDCOUPLING_INSTALL_PYTHON
    ${SKBUILD_PLATLIB_DIR}
    CACHE PATH "Install path: MEDCoupling libs")
  set(MEDCOUPLING_INSTALL_HEADERS
    ${SKBUILD_HEADERS_DIR}
    CACHE PATH "Install path: MEDCoupling headers")
  set(MEDCOUPLING_INSTALL_TESTS
    ${SKBUILD_DATA_DIR}/test
    CACHE PATH "Install path: MEDCoupling tests")
  set(MEDCOUPLING_INSTALL_CMAKE_LOCAL
    ${SKBUILD_DATA_DIR}/cmake_files
    CACHE PATH "Install path: local MEDCoupling CMake files")
  set(MEDCOUPLING_INSTALL_RES
    share/resources
    CACHE PATH "Install path: MEDCoupling resources")
  set(MEDCOUPLING_INSTALL_DOCS
    ${SKBUILD_DATA_DIR}/docs
    CACHE PATH "Install path: MEDCoupling documentation")

  set(MEDCOUPLING_INSTALL_LIBS_DEPS_RPATH $ORIGIN/../medcoupling.libs)
  set(MEDCOUPLING_INSTALL_PYTHON_LIBS_DEPS_RPATH $ORIGIN/medcoupling.libs)
endif()

# Med specific:

set(MEDCOUPLING_INSTALL_RES_DATA
  "${MEDCOUPLING_INSTALL_RES}/med"
  CACHE PATH "Install path: MEDCoupling specific data")

mark_as_advanced(
  MEDCOUPLING_INSTALL_BINS
  MEDCOUPLING_INSTALL_LIBS
  MEDCOUPLING_INSTALL_HEADERS
  MEDCOUPLING_INSTALL_CMAKE_LOCAL
  MEDCOUPLING_INSTALL_RES
  MEDCOUPLING_INSTALL_RES_DATA
  MEDCOUPLING_INSTALL_LIBS_DEPS_RPATH
  MEDCOUPLING_INSTALL_PYTHON_LIBS_DEPS_RPATH
  MEDCOUPLING_INSTALL_DOC)

# TODO: should not be there but in medcoupling_io/medXXX
# for now it stays here because it needs to use this project cmake
# directory when built without conan.
find_package(HDF5 REQUIRED)
find_package(medfile REQUIRED)

if(NOT TARGET HDF5::HDF5)
  if(BUILD_SHARED_LIBS)
    add_library(HDF5::HDF5 ALIAS hdf5::hdf5-shared)
  else()
    add_library(HDF5::HDF5 ALIAS hdf5::hdf5-static)
  endif()
endif()

enable_testing()

add_subdirectory(medcoupling_core)

# TODO: move all this logic specific to shape recogn to the shape recogn CMakeLists.txt
if(MEDCOUPLING_ENABLE_SHAPERECOGN)

  if(UNIX)
    set(CBLAS_ROOT_DIR $ENV{CBLAS_ROOT_DIR} CACHE PATH "Path to the LAPACK/CBLAS.")
    if(CBLAS_ROOT_DIR)
      list(APPEND CMAKE_PREFIX_PATH "${CBLAS_ROOT_DIR}")
      find_package(CBLAS REQUIRED)
    else()
      find_package(BLAS REQUIRED)
    endif()
    set(LAPACK_ROOT_DIR $ENV{LAPACK_ROOT_DIR} CACHE PATH "Path to the LAPACKE")
    if(LAPACK_ROOT_DIR)
      set(LAPACK_ROOT_DIR $ENV{LAPACK_ROOT_DIR} CACHE PATH "Path to the LAPACK.")
      list(APPEND CMAKE_PREFIX_PATH "${LAPACK_ROOT_DIR}")
    endif()
    find_package(LAPACK REQUIRED)
    find_library(LAPACKE_LIB NAMES lapacke REQUIRED)
    set(LAPACK_LIBRARIES ${LAPACKE_LIB} ${LAPACK_LIBRARIES} ${BLAS_LIBRARIES})
    find_path(LAPACKE_INCLUDE_DIRS NAMES lapacke.h HINTS ${LAPACK_LIBRARIES})
    if(LAPACK_FOUND)
      message(STATUS "Lapacke libraries: ${LAPACK_LIBRARIES}")
      message(STATUS "Lapacke include dirs: ${LAPACKE_INCLUDE_DIRS}")
    else()
      message(FATAL_ERROR "Error in Lapacke detection ! lapacke not found !") 
    endif()

  elseif(MSVC)
    set(OPENBLAS_ROOT_DIR $ENV{OPENBLAS_ROOT_DIR} CACHE PATH "Path to the OpenBLAS.")
    if(OPENBLAS_ROOT_DIR)
      list(APPEND CMAKE_PREFIX_PATH "${OPENBLAS_ROOT_DIR}")
      find_package(OpenBLAS REQUIRED)
    else()
      message(FATAL "Could not find OpenBLAS")
    endif()
    set(LAPACK_ROOT_DIR $ENV{LAPACK_ROOT_DIR} CACHE PATH "Path to the LAPACKE")
    if(LAPACK_ROOT_DIR)
      set(LAPACK_ROOT_DIR $ENV{LAPACK_ROOT_DIR} CACHE PATH "Path to the LAPACK.")
      list(APPEND CMAKE_PREFIX_PATH "${LAPACK_ROOT_DIR}")
    endif()
    find_package(LAPACK REQUIRED)
    find_library(LAPACKE_LIB NAMES openblas REQUIRED)
    set(LAPACK_LIBRARIES ${LAPACKE_LIB} ${LAPACK_LIBRARIES} ${BLAS_LIBRARIES})
    find_path(LAPACKE_INCLUDE_DIRS NAMES lapacke.h HINTS ${LAPACK_LIBRARIES})
    if(LAPACK_FOUND)
      message(STATUS "Lapacke libraries: ${LAPACK_LIBRARIES}")
      message(STATUS "Lapacke include dirs: ${LAPACKE_INCLUDE_DIRS}")
    else()
      message(FATAL_ERROR "Error in Lapacke detection ! lapacke not found !") 
    endif()

  else()
    message(FATAL "Not implemented")
  endif()

endif()

if(NOT MEDCOUPLING_MICROMED)
  add_subdirectory(medcoupling_io)
  add_subdirectory(medcoupling_tools)
endif()

if(MEDCOUPLING_ENABLE_PYTHON)
  add_subdirectory(pymedcoupling)
endif()

if(MEDCOUPLING_BUILD_DOC)
  add_subdirectory(doc)
endif()

install(EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${MEDCOUPLING_INSTALL_CMAKE_LOCAL})

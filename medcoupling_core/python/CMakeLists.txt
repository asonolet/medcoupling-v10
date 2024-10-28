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

add_compile_definitions("WITH_NUMPY" "WITH_SCIPY")
# add_compile_definitions("-DNPY_NO_DEPRECATED_API=NPY_1_7_API_VERSION")
# breaks swig

add_subdirectory(medcoupling_core)

if(NOT MEDCOUPLING_MICROMED)
  add_subdirectory(polymesh)
endif()

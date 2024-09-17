add_compile_definitions("WITH_NUMPY" "WITH_SCIPY")
# add_compile_definitions("-DNPY_NO_DEPRECATED_API=NPY_1_7_API_VERSION")
# breaks swig

add_subdirectory(medcoupling_core)

if(NOT MEDCOUPLING_MICROMED)
  add_subdirectory(polymesh)
endif()

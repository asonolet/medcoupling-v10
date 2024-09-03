add_subdirectory(source/sequential)
add_subdirectory(python/sequential)
add_subdirectory(test/sequential)

if(MEDCOUPLING_USE_MPI)
  add_subdirectory(source/para_remapper)
  add_subdirectory(python/para_remapper)
  add_subdirectory(test/para_remapper)
endif()

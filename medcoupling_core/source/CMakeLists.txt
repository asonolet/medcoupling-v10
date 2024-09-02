add_subdirectory(interp_kernel)
add_subdirectory(medcoupling_core)

# medfile_api needs to be splitten in medreader/medwriter (will go into
# medcoupling_io) and polymesh => which is not dependant on Medfile
if(NOT MEDCOUPLING_MICROMED)
  add_subdirectory(medfile_api)
endif()

# TODO: add a library target which links objects targets created in
# subdirectories array, mesh, amr, field, etc

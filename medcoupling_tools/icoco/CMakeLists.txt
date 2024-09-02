add_subdirectory(source)

# if(MEDCOUPLING_ENABLE_PYTHON)
#   find_package(
#     Python
#     COMPONENTS Interpreter Development.Module NumPy
#     REQUIRED)
# 
#   find_package(
#     SWIG
#     COMPONENTS python
#     REQUIRED)
# 
#   include(UseSWIG)
# 
#   list(APPEND CMAKE_SWIG_FLAGS "-DWITH_NUMPY")
#   list(APPEND CMAKE_SWIG_FLAGS "-DWITH_SCIPY")
# 
#   if(MEDCOUPLING_USE_64BIT_IDS)
#     list(APPEND CMAKE_SWIG_FLAGS "-DMEDCOUPLING_USE_64BIT_IDS")
#   endif()
# 
#   add_subdirectory(python)
# endif()

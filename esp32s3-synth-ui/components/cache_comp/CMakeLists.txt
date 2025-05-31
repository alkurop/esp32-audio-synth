# Grab every .cpp under src/
file(GLOB_RECURSE CACHE_SRCS
  "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

idf_component_register(
  SRCS ${CACHE_SRCS}
  INCLUDE_DIRS "include"
  REQUIRES log protocol store
)

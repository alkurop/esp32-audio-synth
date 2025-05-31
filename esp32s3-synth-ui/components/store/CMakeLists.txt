# Grab every .cpp under src/
file(GLOB_RECURSE STORE_SRCS
  "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

idf_component_register(
  SRCS ${STORE_SRCS}
  INCLUDE_DIRS "include"
  REQUIRES log nvs_flash protocol
)

# Grab every .cpp under src/
file(GLOB_RECURSE SENDER_SRCS
  "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

idf_component_register(
  SRCS ${SENDER_SRCS}
  INCLUDE_DIRS "include"
  REQUIRES log protocol
)

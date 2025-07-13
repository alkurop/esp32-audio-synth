# Grab every .cpp under src/
file(GLOB_RECURSE SRC
  "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

idf_component_register(
    SRCS ${SRC}
    INCLUDE_DIRS "include"
    REQUIRES protocol lfo lookup
)

 
# Grab every .cpp under src/
file(GLOB_RECURSE SRCS
  "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "include"
    REQUIRES log protocol  
)


target_compile_options(${COMPONENT_LIB} PRIVATE -O3 -ffast-math -funroll-loops -fstrict-aliasing)

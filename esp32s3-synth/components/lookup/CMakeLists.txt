# Grab every .cpp under src/
file(GLOB_RECURSE SRC
  "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

idf_component_register(
    SRCS ${SRC}
    INCLUDE_DIRS "include"
    REQUIRES protocol
)


# Only add flags if we actually have sources
if(SRC)
    target_compile_options(${COMPONENT_LIB} PRIVATE -O3 -ffast-math -funroll-loops -fstrict-aliasing)
endif()

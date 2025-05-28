# Grab every .cpp under src/
file(GLOB_RECURSE MENU_SRCS
  "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

idf_component_register(
SRCS ${MENU_SRCS}
INCLUDE_DIRS "include"
REQUIRES log driver esp_lvgl_port lvgl menu)



idf_component_register(SRCS 
"display_main.cpp" 
"display_popup.cpp" 
"ssd1306_init.cpp" 
INCLUDE_DIRS "include"
REQUIRES log driver esp_lvgl_port lvgl menu)



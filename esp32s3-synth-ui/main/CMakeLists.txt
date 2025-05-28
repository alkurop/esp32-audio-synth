# Force C++17 in this component
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

idf_component_register(SRCS 
"main.cpp"
INCLUDE_DIRS ""
REQUIRES rotary_pcnt log driver button ssd1306_display menu
)

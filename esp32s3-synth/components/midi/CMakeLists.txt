idf_component_register(SRCS
    "midi_module.cpp"
    "midi_parser.cpp"
    "bpm_counter.cpp"
    INCLUDE_DIRS "include"
    REQUIRES log esp_timer esp_tinyusb)



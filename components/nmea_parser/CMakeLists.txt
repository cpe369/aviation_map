
set(SRCS "")
list(APPEND SRCS
    "nmea_parser.c"
    "nmea_driver.cpp"
)
set(INCLUDE_DIRS "")
list(APPEND INCLUDE_DIRS
    "include"
)   

idf_component_register(SRCS "${SRCS}"
                       INCLUDE_DIRS "${INCLUDE_DIRS}"
                       REQUIRES esp_event esp_driver_uart)
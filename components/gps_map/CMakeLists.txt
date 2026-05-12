set(GPS_MAP_DIR "")
file(GLOB_RECURSE LV_GPS_MAP_SOURCES ${GPS_MAP_DIR}/*.c)
file(GLOB_RECURSE LV_GPS_MAP_SOURCES "images/*.c")

idf_component_register(
    SRCS "gps_map.c" ${LV_GPS_MAP_SOURCES}
    INCLUDE_DIRS "include"
    REQUIRES lvgl esp_system
    PRIV_REQUIRES vfs fatfs
    )



set(SRCS "")
list(APPEND SRCS
    "screen_test.c"
    
)
set(INCLUDE_DIRS "")
list(APPEND INCLUDE_DIRS
    "include"
)   

idf_component_register(SRCS "${SRCS}"
                       INCLUDE_DIRS "${INCLUDE_DIRS}"
                       REQUIRES lvgl esp_driver_gpio esp_driver_i2c bsp_extra nvs_flash esp_common freertos esp_lcd waveshare__esp32_p4_wifi6_touch_lcd_xc)
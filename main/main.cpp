#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_memory_utils.h"
#include "lvgl.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "bsp_board_extra.h"
#include "lv_demos.h"
#include "nmea_driver.h"
#include "screen_test.h"
#include "ui.h"
#include "gps_map.h"


// #define DEG_TO_RAD(deg) ((deg) * M_PI / 180.0)
// #define RAD_TO_DEG(rad) ((rad) * 180.0 / M_PI)

void display_init(void)
{
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = BSP_LCD_DRAW_BUFF_SIZE,
        .double_buffer = BSP_LCD_DRAW_BUFF_DOUBLE,
        .flags = {
            .buff_dma = true,
            .buff_spiram = false,
            .sw_rotate = false,
        }
    };
    bsp_display_start_with_config(&cfg);
    bsp_display_brightness_set(50);
}

void mount_sd(void) {
    esp_err_t err = bsp_sdcard_mount();
    if (err != ESP_OK) {
       // printf("Failed to mount SD card, error: %s\n", esp_err_to_name(err));
        ESP_LOGI("SDCard", "Failed to mount SD card, error: %s\n", esp_err_to_name(err));
    }

    if (!verify_sdcard_mounted()) {
        ESP_LOGI("WARNING", "SD card verification failed!");
    }
    
}

// decouple gps updates
void lvgl_timer(lv_timer_t * timer) {
    
    update_values(get_gps_info());
}

extern "C" void app_main(void)
{   
    display_init();
    mount_sd();
    gps_init();
//   screen_test();

    bsp_display_lock(0);
    
    ui_init();
       
    bsp_display_unlock();

    lv_timer_t * timer = lv_timer_create(lvgl_timer, 10,  NULL);
    (void)timer;

}
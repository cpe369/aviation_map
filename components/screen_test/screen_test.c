#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_memory_utils.h"
#include "lvgl.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "bsp_board_extra.h"
#include "lv_demos.h"
#include "screen_test.h"

// Define colors manually
#define LV_COLOR_RED lv_color_make(0xFF, 0x00, 0x00)
#define LV_COLOR_GREEN lv_color_make(0x00, 0xFF, 0x00)
#define LV_COLOR_BLUE lv_color_make(0x00, 0x00, 0xFF)
#define LV_COLOR_WHITE lv_color_make(0xFF, 0xFF, 0xFF)
#define LV_COLOR_BLACK lv_color_make(0x00, 0x00, 0x00) // Define black color

// Function to fill the screen with a specific color and clear previous content
void fill_screen_with_color(lv_color_t color)
{
    lv_obj_t *scr = lv_scr_act();                             // Get the active screen
    lv_obj_clean(scr);                                        // Clear the screen (removes all objects)
    lv_obj_set_style_bg_color(scr, color, LV_PART_MAIN);      // Set the background color
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN); // Ensure full opacity
}


// Function to handle touch input and draw black points on the screen
void touch_event_handler(lv_event_t *e)
{
    lv_indev_t *indev = lv_indev_get_act(); // Get the active input device
    if (indev == NULL) return; // Check if the input device is valid

    lv_point_t touch_point;
    lv_indev_get_point(indev, &touch_point); // Get the touch point

    // Ensure touch points are within screen boundaries
    if (touch_point.x >= 0 && touch_point.x < 800 && touch_point.y >= 0 && touch_point.y < 1280)
    {
        // Create a small black square (pixel-like) at the touch point
        lv_obj_t *pixel = lv_obj_create(lv_scr_act());
        lv_obj_set_size(pixel, 8, 8);                                   // Set the size for a larger pixel effect
        lv_obj_set_style_radius(pixel, 0, LV_PART_MAIN);                // No rounding for sharp square corners
        lv_obj_set_style_bg_color(pixel, LV_COLOR_BLACK, LV_PART_MAIN); // Set color to black
        lv_obj_set_style_bg_opa(pixel, LV_OPA_COVER, LV_PART_MAIN);     // Set opacity to fully opaque
        lv_obj_set_pos(pixel, touch_point.x, touch_point.y);    // Center the square at the touch point
    }
}

// Function to handle touch area events
void touch_area_event_handler(lv_event_t *e)
{
    // Prevent touch propagation and handle events here
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSING || code == LV_EVENT_PRESSED)
    {
        touch_event_handler(e); // Call the touch event handler
        lv_event_stop_processing(e); // Stop event propagation to prevent scrolling
    }
}

void screen_test(void)
{
    // Fill screen with red, green, blue, white, and black sequentially
    fill_screen_with_color(LV_COLOR_RED);
    vTaskDelay(pdMS_TO_TICKS(1000));

    fill_screen_with_color(LV_COLOR_GREEN);
    vTaskDelay(pdMS_TO_TICKS(1000));

    fill_screen_with_color(LV_COLOR_BLUE);
    vTaskDelay(pdMS_TO_TICKS(1000));

    fill_screen_with_color(LV_COLOR_WHITE);
    vTaskDelay(pdMS_TO_TICKS(1000));

    fill_screen_with_color(LV_COLOR_BLACK);
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Set up touch area to capture touch events
    lv_obj_t *touch_area = lv_obj_create(lv_scr_act());
    lv_obj_set_size(touch_area, 800, 1280); // Full screen size
    lv_obj_set_style_bg_opa(touch_area, LV_OPA_TRANSP, LV_PART_MAIN); // Transparent background
    lv_obj_add_event_cb(touch_area, touch_area_event_handler, LV_EVENT_ALL, NULL); // Add event handler

     // Disable scrolling for the entire screen
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLL_CHAIN_VER);

    ESP_LOGI("screen_test", "Screen test completed.");
}

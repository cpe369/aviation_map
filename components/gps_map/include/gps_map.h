#pragma once

#include "lvgl.h"

#define MAP_TILES_TILE_SIZE 256
#define MAP_TILES_GRID_COLS 3
#define MAP_TILES_GRID_ROWS 3
#define MAP_TILES_BYTES_PER_PIXEL 2
#define MAP_TILES_COLOR_FORMAT LV_COLOR_FORMAT_RGB565
#define MAP_TILES_CONTAINER_WIDTH (MAP_TILES_TILE_SIZE * MAP_TILES_GRID_COLS)
#define MAP_TILES_CONTAINER_HEIGHT (MAP_TILES_TILE_SIZE * MAP_TILES_GRID_ROWS)
#define MAP_HEIGHT 655  // Adjusted for circular map display
#define MAP_WIDTH 655  // Adjusted for circular map display
#define MAP_ZOOM 12
#define BASE_PATH "/sdcard"
#define TILE_FOLDER "tiles1"
#define STEP_ANIMATION_DURATION 500

#ifdef __cplusplus
extern "C" {
#endif

// Function declarations
bool verify_sdcard_mounted(void);
bool GPSMap_init(lv_obj_t* parent_screen);
void GPSMap_update(double latitude, double longitude);
void GPSMap_rotate(float heading);
void cleanup();
void set_zoom_level(int zoom);

//extern lv_obj_t** tile_components;

#ifdef __cplusplus
}
#endif
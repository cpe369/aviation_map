#include <stdio.h>
#include "gps_map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define TAG "GPSMap"

#define DEG_TO_RAD(deg) ((deg) * M_PI / 180.0)
#define RAD_TO_DEG(rad) ((rad) * 180.0 / M_PI)

// Tile slot structure definition
typedef struct {
    uint8_t* buf;          // Pixel buffer
    lv_image_dsc_t img;    // LVGL image descriptor
} TileSlot;

static TileSlot tiles[MAP_TILES_GRID_ROWS * MAP_TILES_GRID_COLS];

lv_obj_t* map_container;
lv_obj_t* map_group;
lv_obj_t** tile_components;
lv_obj_t * ui_cur_loc_mkr;
lv_obj_t * ui_cen_loc_mkr;
lv_obj_t * ui_img161364;
lv_obj_t * ui_img162364;
lv_obj_t * ui_img163364;
lv_obj_t * ui_img161365;
lv_obj_t * ui_img162365;
lv_obj_t * ui_img163365;
lv_obj_t * ui_img161366;
lv_obj_t * ui_img162366;
lv_obj_t * ui_img163366;

//static int tile_count = 0;
static int top_left_tile_x = 0;
static int top_left_tile_y = 0;
static int new_top_left_tile_x = 0;
static int new_top_left_tile_y = 0;
static int marker_offset_x = 0;
static int marker_offset_y = 0;
static int new_marker_offset_x = 0;
static int new_marker_offset_y = 0;
static bool initialized = false;
static bool is_loading = false;
static int zoom_level = MAP_ZOOM;

bool receiving_data           = false; // has the first data been received
volatile bool data_ready      = false; // new incoming data
bool init_anim_complete       = false; // needle sweep completed - tbc
bool location_initialized     = false; // has the initial GPS location been set


#pragma region  unsued_functions
void create_center_location_marker() {
    ui_cen_loc_mkr = lv_obj_create(map_container);
    lv_obj_set_width(ui_cen_loc_mkr, 20);
    lv_obj_set_height(ui_cen_loc_mkr, 20);
    lv_obj_set_x(ui_cen_loc_mkr, 0);
    lv_obj_set_y(ui_cen_loc_mkr, 0);
    lv_obj_set_align(ui_cen_loc_mkr, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_cen_loc_mkr, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_cen_loc_mkr, lv_color_hex(0xFCAF00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_cen_loc_mkr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
}

#pragma endregion


void shift_right() {
    // Implement right shift logic here
}
void shift_left() {
    // Implement left shift logic here
}
void shift_down() {
    // Implement down shift logic here
}
void shift_up() {
    // Implement up shift logic here
}

void show_loading_popup() {
    // Implement popup display logic here
}
void hide_loading_popup() {
    // Implement popup hide logic here
}


void animate_map_center() {
    // Implement smooth animation logic here
}

void set_zoom_level(int zoom) {
    zoom_level = zoom;
    // Implement zoom level adjustment logic here
}



void cleanup() {
    if (!initialized) {
        return;
    }

    // Free tile components and buffers
    for (int i = 0; i < MAP_TILES_GRID_ROWS * MAP_TILES_GRID_COLS; i++) {
        if (tiles[i].buf) {
            free(tiles[i].buf);
            tiles[i].buf = NULL;
        }
    }

    if (tile_components) {
        free(tile_components);
        tile_components = NULL;
    }

    // Delete LVGL objects
    if (map_group) {
        lv_obj_del(map_group);
        map_group = NULL;
    }
    if (map_container) {
        lv_obj_del(map_container);
        map_container = NULL;
    }

    initialized = false;
}


bool is_location_within_center(int new_latitude, int new_longitude) {
    return (new_latitude == top_left_tile_x) && (new_longitude == top_left_tile_y);
}

void get_tile_coordinates(double latitude, double longitude, double *tile_x, double *tile_y) {
    // Convert latitude and longitude to tile coordinates at the specified zoom level
    double lat_rad = DEG_TO_RAD(latitude);
    int n = 1 << zoom_level;
    *tile_x = n * ((longitude + 180.0) / 360.0);
    *tile_y = n * (1.0 - (log(tan(lat_rad) + 1.0 / cos(lat_rad)) / M_PI)) / 2.0;
 //   ESP_LOGI("GPSMap", "Tile Coordinates: X=%.5f, Y=%.5f", *tile_x, *tile_y);
}

void lat_long_to_pixel_offset(double latitude, double longitude, double center_latitude, double center_longitude, int *pixel_x, int *pixel_y) {
    // Convert lat/long coordinates to pixel offsets from the map center
    // Returns pixel distance from map center (0,0 = center, negative = up/left, positive = down/right)
    
    double tile_x, tile_y;
    double center_tile_x, center_tile_y;
    
    // Get tile coordinates for both the target point and the center
    get_tile_coordinates(latitude, longitude, &tile_x, &tile_y);
    get_tile_coordinates(center_latitude, center_longitude, &center_tile_x, &center_tile_y);
    
    // Calculate the tile difference in pixels (each tile is MAP_TILES_TILE_SIZE pixels)
    // Fractional part of tile coordinates represents position within the tile (0.0 to 1.0)
    double tile_diff_x = (tile_x - center_tile_x) * MAP_TILES_TILE_SIZE;
    double tile_diff_y = (tile_y - center_tile_y) * MAP_TILES_TILE_SIZE;
    
    // Convert to integers and center at (0, 0)
    *pixel_x = (int)round(tile_diff_x);
    *pixel_y = (int)round(tile_diff_y);
}

void center_map_on_gps(double latitude, double longitude) {
    
    double tile_x, tile_y;
    get_tile_coordinates(latitude, longitude, &tile_x, &tile_y);
    
    // Calculate which tile in the grid (0, 1, or 2 for 3x3 grid)
    int tile_col = (int)floor(tile_x) - top_left_tile_x;
    int tile_row = (int)floor(tile_y) - top_left_tile_y;

    int offset_x = (tile_col * MAP_TILES_TILE_SIZE) + ( marker_offset_x - (MAP_TILES_CONTAINER_WIDTH / 2));
    int offset_y = (tile_row * MAP_TILES_TILE_SIZE) + ( marker_offset_y - (MAP_TILES_CONTAINER_HEIGHT / 2));
   
    lv_obj_align(map_group, LV_ALIGN_CENTER, 0 - offset_x, 0 - offset_y);
    ESP_LOGI("GPSMap", "Centering map with offsets: X=%d, Y=%d", offset_x, offset_y);

//     lv_obj_get_style_transform_rotation(map_group, LV_PART_MAIN);
//     // 1. Set Pivot to Center (crucial for proper rotation)
//     lv_obj_set_style_transform_pivot_x(map_group, MAP_TILES_CONTAINER_WIDTH/2, LV_PART_MAIN); // 50 = Width/2
//     lv_obj_set_style_transform_pivot_y(map_group, MAP_TILES_CONTAINER_HEIGHT/2, LV_PART_MAIN); // 25 = Height/2

// // 2. Set Rotation Angle (e.g., 45 degrees)
// // 450 means 45.0 degrees (0.1 deg unit)
//     lv_obj_set_style_transform_angle(map_group, 450, LV_PART_MAIN);

}

void create_current_location_marker(double latitude, double longitude) {

    // Calculate total offset accounting for tile grid position
    // The marker is within the tile grid, so we need to account for:
    // 1. Which tile in the 3x3 grid (top_left_tile_x/Y)
    // 2. Position within that tile (marker_offset_x/Y)
    
    double tile_x, tile_y;
    get_tile_coordinates(latitude, longitude, &tile_x, &tile_y);
    
    // Calculate which tile in the grid (0, 1, or 2 for 3x3 grid)
    int tile_col = (int)floor(tile_x) - top_left_tile_x;
    int tile_row = (int)floor(tile_y) - top_left_tile_y;
    
    // Total offset = (tile position in grid * tile size) + position within tile
    int offset_x = (tile_col * MAP_TILES_TILE_SIZE) + ( marker_offset_x - (MAP_TILES_CONTAINER_WIDTH / 2));
    int offset_y = (tile_row * MAP_TILES_TILE_SIZE) + ( marker_offset_y - (MAP_TILES_CONTAINER_HEIGHT / 2));

    ui_cur_loc_mkr = lv_obj_create(map_container);
    lv_obj_set_width(ui_cur_loc_mkr, 20);
    lv_obj_set_height(ui_cur_loc_mkr, 20);

    // Position marker at calculated offset from map center
    // lv_obj_set_x(ui_cur_loc_mkr, offset_x);
    // lv_obj_set_y(ui_cur_loc_mkr, offset_y);

    // For initial placement, set to (0,0) and align to center.
    lv_obj_set_x(ui_cur_loc_mkr, 0);
    lv_obj_set_y(ui_cur_loc_mkr,0);

    lv_obj_set_align(ui_cur_loc_mkr, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_cur_loc_mkr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_cur_loc_mkr, lv_color_hex(0x0000FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_cur_loc_mkr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

}

void set_fallback_tile(int col) {
    for (int row = 0; row < MAP_TILES_GRID_ROWS; row++) {
        int index = row * MAP_TILES_GRID_COLS + col;
        // Fill with solid grey color (RGB565: 0x8410 = grey)
        uint16_t grey_rgb565 = 0x8410;  // RGB565 grey color
        uint16_t* buf_16 = (uint16_t*)tiles[index].buf;
        for (int i = 0; i < MAP_TILES_TILE_SIZE * MAP_TILES_TILE_SIZE; i++) {
            buf_16[i] = grey_rgb565;
        }
        lv_image_set_src(tile_components[index], &tiles[index].img);
    }
}

bool fetch_images_from_sd(int index, int tile_x, int tile_y) {
    // Check if tile_components is initialized
    if (!tile_components) {
        ESP_LOGI("GPSMap", "Error: tile_components not initialized");
        return false;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), BASE_PATH "/" TILE_FOLDER "/%d/%d/%d.bin", zoom_level, tile_x, tile_y);
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        ESP_LOGI("GPSMap", "Failed to open tile image: %s", filepath);
        return false;
    }

    // Skip 12-byte header 
    fseek(file, 12, SEEK_SET);

    size_t read_bytes = fread(tiles[index].buf, 1, tiles[index].img.data_size, file);
    fclose(file);

    if (read_bytes != tiles[index].img.data_size) {
        ESP_LOGI("GPSMap", "Incomplete read for tile image: %s (got %zu, expected %lu)", filepath, read_bytes, tiles[index].img.data_size);
        return false;
    }

    lv_image_set_src(tile_components[index], &tiles[index].img);
    ESP_LOGI("GPSMap", "Loaded tile image: %s", filepath);
    return true;
}

bool load_tile_images() {
    for (int row = 0; row < MAP_TILES_GRID_ROWS; row++) {
        for (int col = 0; col < MAP_TILES_GRID_COLS; col++) {
            int index = row * MAP_TILES_GRID_COLS + col;
            int tile_x = top_left_tile_x + col;
            int tile_y = top_left_tile_y + row;
            ESP_LOGI("GPSMap", "Loading tile at index %d (Tile X: %d, Tile Y: %d)", index, tile_x, tile_y);
            if (!fetch_images_from_sd(index, tile_x, tile_y)) {
                ESP_LOGI("GPSMap", "Using fallback for tile (%d, %d)", tile_x, tile_y);
                set_fallback_tile(col);
            }
        }
    }
    return true;
}

// Calculate pixel offsets within a tile for the marker
void get_marker_offsets(double *tile_x, double *tile_y, int *offset_x, int *offset_y) {
    double fractional_x = *tile_x - floor(*tile_x);
    double fractional_y = *tile_y - floor(*tile_y);
    *offset_x = (int)(fractional_x * MAP_TILES_TILE_SIZE);
    *offset_y = (int)(fractional_y * MAP_TILES_TILE_SIZE);
 //   ESP_LOGI("GPSMap", "Marker Offsets: X=%d, Y=%d", *offset_x, *offset_y);
}

void update_location(double latitude, double longitude) {
   static int current_zoom = MAP_ZOOM;
   
    if (!initialized || is_loading) {
        return;
    }
    
    double tile_x, tile_y;
    get_tile_coordinates(latitude, longitude, &tile_x, &tile_y);
    get_marker_offsets(&tile_x, &tile_y, &new_marker_offset_x, &new_marker_offset_y);

    new_top_left_tile_x = (int)(floor(tile_x)) - 1;
    new_top_left_tile_y = (int)(floor(tile_y)) - 1;

    if (zoom_level != current_zoom || !is_location_within_center(new_top_left_tile_x, new_top_left_tile_y))
    {
         // Need to shift tiles
        is_loading = true;
        show_loading_popup();

        // Update top-left tile indices
        top_left_tile_x = new_top_left_tile_x;
        top_left_tile_y = new_top_left_tile_y;

        // Load new tiles
        load_tile_images();

        // Center map on GPS location
        center_map_on_gps(latitude, longitude);

        hide_loading_popup();
        is_loading = false;

        current_zoom = zoom_level;
    }
    else {
        // No zoom change, just animate marker
        animate_map_center();
    }
        
}

void show_initial_location(double latitude, double longitude) {
    if (!initialized) {
        return;
    }

    // Find center tile coordinates
    ESP_LOGI("GPSMap", "Showing initial location: Lat=%.5f, Lon=%.5f", latitude, longitude);
    double tile_x, tile_y;
    get_tile_coordinates(latitude, longitude, &tile_x, &tile_y);
    ESP_LOGI("GPSMap", "Initial Tile Coordinates: X=%.2f, Y=%.2f", tile_x, tile_y);

    get_marker_offsets(&tile_x, &tile_y, &marker_offset_x, &marker_offset_y);

    // Calculate top-left tile coordinates based on center tile
    top_left_tile_x = (int)(floor(tile_x)) - 1;
    top_left_tile_y = (int)(floor(tile_y)) - 1;

    // Load tiles
    load_tile_images();

    // Center map on GPS location
    center_map_on_gps(latitude, longitude);
    //GPSMap_rotate(180.0f); // Initial rotation to South
    create_current_location_marker(latitude, longitude);
    //create_center_location_marker();
    location_initialized = true;
    ESP_LOGI("GPSMap", "Initial location set on map.");
}

void GPSMap_update(double latitude, double longitude) {
    if (!initialized) {
        return;
    }
    if(location_initialized == false) {
        show_initial_location(latitude, longitude);
        location_initialized = true;
        return;
    }   
    else {  
       update_location(latitude, longitude);
    }
   
}

void GPSMap_rotate(float heading) {
    // Rotate map container to heading
    // heading: 0-360 degrees (0° = North = no rotation)
    
    if (!initialized || !map_container) {
        return;
    }
    
    // Normalize heading to 0-360 range
    while (heading < 0) heading += 360.0f;
    while (heading >= 360) heading -= 360.0f;
    int angle = (int)(heading * 10); // LVGL uses 0.1 degree units

    
    ESP_LOGI(TAG, "Map rotation to heading: %.1f° ", heading);

    lv_obj_get_style_transform_rotation(map_group, LV_PART_MAIN);
    // 1. Set Pivot to Center (crucial for proper rotation)
    lv_obj_set_style_transform_pivot_x(map_group, MAP_TILES_CONTAINER_WIDTH/2, LV_PART_MAIN); 
    lv_obj_set_style_transform_pivot_y(map_group, MAP_TILES_CONTAINER_HEIGHT/2, LV_PART_MAIN); 

    // 2. Set Rotation Angle (e.g., 45 degrees)
    // 450 means 45.0 degrees (0.1 deg unit)
    lv_obj_set_style_transform_angle(map_group, angle, LV_PART_MAIN);
}

void create_tile_components() {
    tile_components = (lv_obj_t**)malloc(sizeof(lv_obj_t*) * MAP_TILES_GRID_ROWS * MAP_TILES_GRID_COLS);
    for (int row = 0; row < MAP_TILES_GRID_ROWS; row++) {
        for (int col = 0; col < MAP_TILES_GRID_COLS; col++) {
            int index = row * MAP_TILES_GRID_COLS + col;
            tile_components[index] = lv_image_create(map_group);
            lv_obj_set_size(tile_components[index], MAP_TILES_TILE_SIZE, MAP_TILES_TILE_SIZE);
            // Overlap tiles by 1 pixel to hide seams
            lv_obj_set_x(tile_components[index], col * (MAP_TILES_TILE_SIZE));
            lv_obj_set_y(tile_components[index], row * (MAP_TILES_TILE_SIZE));
            lv_obj_set_style_opa(tile_components[index], LV_OPA_COVER, 0);
            lv_obj_set_style_bg_color(tile_components[index], lv_color_black(), 0);
            lv_obj_set_style_pad_all(tile_components[index], 0, 0);
            lv_obj_set_style_border_width(tile_components[index], 0, 0);

            // Initialize tile slot
            tiles[index].buf = (uint8_t*)malloc(MAP_TILES_TILE_SIZE * MAP_TILES_TILE_SIZE * MAP_TILES_BYTES_PER_PIXEL);
            memset(tiles[index].buf, 0xFF, MAP_TILES_TILE_SIZE * MAP_TILES_TILE_SIZE * MAP_TILES_BYTES_PER_PIXEL); // White fallback

            tiles[index].img.header.w = MAP_TILES_TILE_SIZE;
            tiles[index].img.header.h = MAP_TILES_TILE_SIZE;
            tiles[index].img.header.cf = MAP_TILES_COLOR_FORMAT;
            tiles[index].img.data_size = MAP_TILES_TILE_SIZE * MAP_TILES_TILE_SIZE * MAP_TILES_BYTES_PER_PIXEL;
            tiles[index].img.data = tiles[index].buf;

            // Set initial image to white
            lv_image_set_src(tile_components[index], &tiles[index].img);
        }
    }
}

bool GPSMap_init(lv_obj_t* parent_screen) {
    if (initialized) {
        return true;
    }
    // Create map container
    map_container = lv_obj_create(parent_screen);
    lv_obj_remove_style_all(map_container);
    //lv_obj_set_size(map_container, MAP_WIDTH, MAP_HEIGHT);
    lv_obj_set_width(map_container, MAP_WIDTH);
    lv_obj_set_height(map_container, MAP_HEIGHT);
    lv_obj_set_align(map_container, LV_ALIGN_CENTER);
    lv_obj_set_style_radius(map_container, 400, 0);
    lv_obj_set_style_bg_color(map_container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(map_container, 0, 0);
    lv_obj_set_style_clip_corner(map_container, true, 0);
    lv_obj_set_style_border_width(map_container, 5, 0);
    lv_obj_set_style_outline_color(map_container, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_outline_opa(map_container, 255, 0);
    lv_obj_set_style_outline_width(map_container, 5, 0);
    lv_obj_set_style_outline_pad(map_container, 0, 0);

    // Create map group
    map_group = lv_obj_create(map_container);
    lv_obj_remove_style_all(map_group);
    //lv_obj_set_size(map_group, 768, 768);  //  Make it larger to hold 3x3 tiles
    lv_obj_set_width(map_group, MAP_TILES_CONTAINER_WIDTH);
    lv_obj_set_height(map_group, MAP_TILES_CONTAINER_HEIGHT);
    lv_obj_set_align(map_group, LV_ALIGN_CENTER);
    lv_obj_set_style_opa(map_group, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(map_group, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(map_group, 0, 0);

     create_tile_components();
    // create_static_tile_components();

    initialized = true;
    return true;
}

// Verify SD card is mounted and accessible
bool verify_sdcard_mounted(void) {
    struct stat st;
    
    // Check if mount point exists
    if (stat(BASE_PATH, &st) != 0) {
        ESP_LOGE(TAG, "SD card mount point does not exist: %s", BASE_PATH);
        return false;
    }
    
    // Check if tile folder exists
    char tile_path[256];
    snprintf(tile_path, sizeof(tile_path), "%s/%s", BASE_PATH, TILE_FOLDER);
    if (stat(tile_path, &st) != 0) {
        ESP_LOGE(TAG, "Tile folder does not exist: %s", tile_path);
        return false;
    }
    
    // Try to open a test file in the tile folder
    char test_file[256];
    snprintf(test_file, sizeof(test_file), "%s/%s/test.txt", BASE_PATH, TILE_FOLDER);
    FILE* f = fopen(test_file, "r");
    if (f) {
        fclose(f);
        ESP_LOGI(TAG, "SD card verified - test file found");
    } else {
        ESP_LOGW(TAG, "SD card mounted but test file not found at: %s", test_file);
    }
    
    ESP_LOGI(TAG, "SD card mounted and accessible at: %s", BASE_PATH);
    return true;
}



void deinit() {
    cleanup();
}











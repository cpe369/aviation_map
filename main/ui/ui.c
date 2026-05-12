

#include "ui.h"
#include "gps_map.h"
#include <math.h>
#include "esp_log.h"

///////////////////// VARIABLES ////////////////////
// screens
lv_obj_t *ui_main_scr;

// global elements
lv_obj_t *ui_compass;

lv_obj_t *ui_heading_bug;
lv_obj_t * ui_Container1 = NULL;
lv_obj_t * ui_lbllatitude = NULL;
lv_obj_t * ui_lbllongitude = NULL;
lv_obj_t * ui_lblcourse = NULL;
lv_obj_t * ui_Container2 = NULL;
lv_obj_t * ui_lbllatitudeval = NULL;
lv_obj_t * ui_lbllongitudeval = NULL;
lv_obj_t * ui_lblcourseval = NULL;
lv_obj_t * ui_zoom = NULL;
lv_obj_t * ui_upper_txt = NULL;

// general color palettes
const lv_color_t PALETTE_BLACK        = LV_COLOR_MAKE(0, 0, 0);
const lv_color_t PALETTE_ORANGE       = LV_COLOR_MAKE(244, 153, 37);
const lv_color_t PALETTE_GREY         = LV_COLOR_MAKE(120, 120, 120);
const lv_color_t PALETTE_WHITE        = LV_COLOR_MAKE(255, 255, 255);

static float new_latitude            = 0.0;
static float new_longitude           = 0.0;
static float course                  = 0.0f; // Replace with actual course calculation if available
// EVENTS
lv_obj_t * ui____initial_actions0;

// IMAGES AND IMAGE SETS

///////////////////// ANIMATIONS ////////////////////


// Callback function for the animation
void arrow_anim_cb(void * var, int32_t v) {
    lv_obj_t * arrow = (lv_obj_t *)var;
    
    int32_t radius = 385;    // Distance from center
    int32_t center_x = 0;  // X coordinate of center point
    int32_t center_y = 0;  // Y coordinate of center point
    
    // 1. Calculate Position (v is angle in 0.1 deg, e.g., 0 to 3600)
    // We divide by 10 because lv_trigo functions take degrees, but our 'v' has 0.1 precision
    int32_t x = center_x + (radius * lv_trigo_cos(v / 10)) / LV_TRIGO_SIN_MAX;
    int32_t y = center_y + (radius * lv_trigo_sin(v / 10)) / LV_TRIGO_SIN_MAX;
    
    lv_obj_set_pos(arrow, x, y);

    // 2. Set Rotation to point inward
    // If your arrow image points UP by default, adding 2700 (270 deg) 
    // to the current position angle will make it point to the center.
    // Adjust the '+ 2700' offset based on your specific image's orientation.
    lv_img_set_angle(arrow, v + 2700); 
}

void move_arrow_to_heading(lv_obj_t * arrow, int32_t target_deg) {
    float heading = 0.0f; // Replace with actual heading value if available
     // Normalize heading to 0-360 range
    while (heading < 0) heading += 360.0f;
    while (heading >= 360) heading -= 360.0f;
    
    // 1. Get current position (you'll need to store this or extract it)
    // For simplicity, let's assume you store 'current_angle' globally or in user_data
    static int32_t current_angle = 0; 

    // 2. Shortest Path Logic (Optional but recommended)
    // Ensures the arrow doesn't spin 300 degrees to move 60 degrees
    int32_t diff = target_deg - current_angle;
    if (diff > 1800) diff -= 3600;
    if (diff < -1800) diff += 3600;
    int32_t end_angle = current_angle + diff;

    // 3. Configure the one-shot animation
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, arrow);
    lv_anim_set_exec_cb(&a, arrow_anim_cb); // Use the same callback from before
    
    lv_anim_set_values(&a, current_angle, end_angle);
    lv_anim_set_time(&a, 1000); // 1 second to reach new heading
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out); // Smooth deceleration
    
    // Ensure repeat is disabled (default for new anims)
    lv_anim_set_repeat_count(&a, 1); 
    
    lv_anim_start(&a);

    // Update current_angle for the next call (keeping it within 0-360)
    current_angle = (end_angle + 3600) % 3600;
}

// Start the animation
void start_arrow_animation(lv_obj_t * arrow) {

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, arrow);
    lv_anim_set_values(&a, 0, 3600); // Full 360 degree rotation
    lv_anim_set_time(&a, 3000);     // 3 seconds per loop
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&a, arrow_anim_cb);
    lv_anim_start(&a);
}

void test_animte() {
    if (ui_heading_bug) {
       // start_arrow_animation(ui_heading_bug);
        move_arrow_to_heading(ui_heading_bug, 900); // Move to 90.0 degrees (East)
    }
}

///////////////////// FUNCTIONS ////////////////////
void update_gps_labels(float latitude, float longitude, float course) {
    // Update latitude
    char lat_str[16];
    snprintf(lat_str, sizeof(lat_str), "%.5f", latitude);
    lv_label_set_text(ui_lbllatitudeval, lat_str);

    // Update longitude
    char lon_str[16];
    snprintf(lon_str, sizeof(lon_str), "%.5f", longitude);
    lv_label_set_text(ui_lbllongitudeval, lon_str);

    // Update course
    char course_str[16];
    snprintf(course_str, sizeof(course_str), "%.2f", course);
    lv_label_set_text(ui_lblcourseval, course_str);
}

void update_values(gps_t gps_data) {
    new_latitude = gps_data.latitude;
    new_longitude = gps_data.longitude;
    course = -90;// = gps_data.cog;
    static bool heading_init = false;
    int heading = (gps_data.cog * 10); // in 0.1 deg
    //update_gps_labels(new_latitude, new_longitude, course);

    if (gps_data.valid)
    {
        GPSMap_update((double)new_latitude, (double)new_longitude);
        
        //Update heading only if speed is greater than 10 m/s
        if (gps_data.speed > 10.0f && !heading_init) {
            move_arrow_to_heading(ui_heading_bug, heading);
        }
        else if (!heading_init)
        {
            move_arrow_to_heading(ui_heading_bug, -900);
            heading_init = true;
        }
    }
}

void animate_heading_bug(float heading) {
    // Animate the heading bug around the compass rose
    // heading: 0-360 degrees (0° = North, 90° = East, 180° = South, 270° = West)
    
    if (!ui_heading_bug || !ui_compass) {
        ESP_LOGI("UI", "Bug or compass null!");
        return;
    }
    
    // Normalize heading to 0-360 range
    while (heading < 0) heading += 360.0f;
    while (heading >= 360) heading -= 360.0f;
    
    // Get heading bug dimensions
    int bug_width = lv_obj_get_width(ui_heading_bug);
    int bug_height = lv_obj_get_height(ui_heading_bug);
    
    // Get compass actual position and size (it uses LV_ALIGN_CENTER)
    int compass_x = lv_obj_get_x(ui_compass);  //0
    int compass_y = lv_obj_get_y(ui_compass);  //0
    int compass_width = lv_obj_get_width(ui_compass); //800
    int compass_height = lv_obj_get_height(ui_compass);//800
    // ESP_LOGI("UI", "Compass Pos: (%d, %d), Size: (%d, %d)", compass_x, compass_y, compass_width, compass_height);

    // Compass center point in screen coordinates
    int compass_center_x = compass_x + compass_width / 2; //400
    int compass_center_y = compass_y + compass_height / 2; //400
    // ESP_LOGI("UI", "Compass Center: (%d, %d)", compass_center_x, compass_center_y);

    // Bug position radius from compass center
    int bug_radius = 350;
    
    // Convert heading to radians (0° = top/north = -90° in standard math coords)
    float angle_rad = (heading - 90.0f) * M_PI / 180.0f;
    
    // Calculate bug center position
    int bug_center_x = compass_center_x + (int)(bug_radius * cosf(angle_rad)); //750
    int bug_center_y = compass_center_y + (int)(bug_radius * sinf(angle_rad)); //400
   // ESP_LOGI("UI", "Bug Center: (%d, %d)", bug_center_x, bug_center_y);

    // Offset to top-left corner for positioning
    int bug_x = bug_center_x - bug_width / 2; //722
    int bug_y = bug_center_y - bug_height / 2; //382
    
   // ESP_LOGI("UI", "Heading: %.1f°, Bug: (%d, %d)", heading, bug_x, bug_y);
    
    // Set position directly (animations can interfere with visibility)
    lv_obj_set_x(ui_heading_bug, bug_x);
    lv_obj_set_y(ui_heading_bug, bug_y);
    
    // TODO: Add rotation for arrow to point inward
    // LVGL 9.3 rotation API to be determined
}

///////////////////// SCREENS ////////////////////


// Test screen to display GPS data
void gps_data(void)
{
    ui_Container1 = lv_obj_create(ui_main_scr);
    lv_obj_remove_style_all(ui_Container1);
    lv_obj_set_width(ui_Container1, 215);
    lv_obj_set_height(ui_Container1, 130);
    lv_obj_set_x(ui_Container1, 150);
    lv_obj_set_y(ui_Container1, 340);
    lv_obj_set_flex_flow(ui_Container1, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_Container1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_remove_flag(ui_Container1, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_Container1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Container1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Container1, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_lbllatitude = lv_label_create(ui_Container1);
    lv_obj_set_width(ui_lbllatitude, LV_SIZE_CONTENT);   
    lv_obj_set_height(ui_lbllatitude, LV_SIZE_CONTENT);    
    lv_obj_set_x(ui_lbllatitude, -13);
    lv_obj_set_y(ui_lbllatitude, -42);
    lv_obj_set_align(ui_lbllatitude, LV_ALIGN_CENTER);
    lv_label_set_text(ui_lbllatitude, "Latitude: ");
    lv_obj_set_style_text_color(ui_lbllatitude, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_lbllatitude, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_lbllatitude, &lv_font_montserrat_38, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_lbllongitude = lv_label_create(ui_Container1);
    lv_obj_set_width(ui_lbllongitude, LV_SIZE_CONTENT);  
    lv_obj_set_height(ui_lbllongitude, LV_SIZE_CONTENT);    
    lv_obj_set_x(ui_lbllongitude, -5);
    lv_obj_set_y(ui_lbllongitude, -17);
    lv_obj_set_align(ui_lbllongitude, LV_ALIGN_CENTER);
    lv_label_set_text(ui_lbllongitude, "Longitude:");
    lv_obj_set_style_text_color(ui_lbllongitude, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_lbllongitude, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_lbllongitude, &lv_font_montserrat_38, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_lblcourse = lv_label_create(ui_Container1);
    lv_obj_set_width(ui_lblcourse, LV_SIZE_CONTENT);  
    lv_obj_set_height(ui_lblcourse, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_lblcourse, -16);
    lv_obj_set_y(ui_lblcourse, 6);
    lv_obj_set_align(ui_lblcourse, LV_ALIGN_CENTER);
    lv_label_set_text(ui_lblcourse, "Course:");
    lv_obj_set_style_text_color(ui_lblcourse, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_lblcourse, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_lblcourse, &lv_font_montserrat_38, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Container2 = lv_obj_create(ui_main_scr);
    lv_obj_remove_style_all(ui_Container2);
    lv_obj_set_width(ui_Container2, 250);
    lv_obj_set_height(ui_Container2, 130);
    lv_obj_set_x(ui_Container2, 365);
    lv_obj_set_y(ui_Container2, 340);
    lv_obj_set_flex_flow(ui_Container2, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_Container2, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_remove_flag(ui_Container2, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_Container2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Container2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Container2, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_lbllatitudeval = lv_label_create(ui_Container2);
    lv_obj_set_width(ui_lbllatitudeval, LV_SIZE_CONTENT); 
    lv_obj_set_height(ui_lbllatitudeval, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_lbllatitudeval, -11);
    lv_obj_set_y(ui_lbllatitudeval, -17);
    lv_obj_set_align(ui_lbllatitudeval, LV_ALIGN_CENTER);
    lv_label_set_text(ui_lbllatitudeval, "0.000");
    lv_obj_set_style_text_color(ui_lbllatitudeval, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_lbllatitudeval, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_lbllatitudeval, &lv_font_montserrat_38, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_lbllongitudeval = lv_label_create(ui_Container2);
    lv_obj_set_width(ui_lbllongitudeval, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_lbllongitudeval, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_lbllongitudeval, -11);
    lv_obj_set_y(ui_lbllongitudeval, -17);
    lv_obj_set_align(ui_lbllongitudeval, LV_ALIGN_CENTER);
    lv_label_set_text(ui_lbllongitudeval, "0.000");
    lv_obj_set_style_text_color(ui_lbllongitudeval, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_lbllongitudeval, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_lbllongitudeval, &lv_font_montserrat_38, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_lblcourseval = lv_label_create(ui_Container2);
    lv_obj_set_width(ui_lblcourseval, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_lblcourseval, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_lblcourseval, -11);
    lv_obj_set_y(ui_lblcourseval, -17);
    lv_obj_set_align(ui_lblcourseval, LV_ALIGN_CENTER);
    lv_label_set_text(ui_lblcourseval, "000.0");
    lv_obj_set_style_text_color(ui_lblcourseval, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_lblcourseval, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_lblcourseval, &lv_font_montserrat_38, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void zoom_event_cb(lv_event_t * e)
{

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    int32_t value = lv_arc_get_value(obj);

    int32_t step = 1; // Snap to 0, 10, 20...
    int32_t snapped = ((value + (step / 2)) / step) * step;
    lv_arc_set_value(obj, snapped);

    if (code == LV_EVENT_VALUE_CHANGED) {
        // This prints continuously while dragging
        ESP_LOGI("UI", "Zoom value changed to: %d", lv_arc_get_value(obj));
        set_zoom_level(value);
    }
    else if (code == LV_EVENT_RELEASED) {
        // This prints once when released
        ESP_LOGI("UI", "Zoom slider released at value: %d", lv_arc_get_value(obj));
        // Perform action with final value here
        //set_zoom_level(value);
    }
}

void ui_mainscreen_screen_init(void) {

    ui_main_scr = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_main_scr, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_main_scr, lv_color_hex(0x020202), 0);
    lv_obj_set_style_bg_opa(ui_main_scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_compass = lv_image_create(ui_main_scr);
    lv_image_set_src(ui_compass, &compass);
    lv_obj_set_width(ui_compass, LV_SIZE_CONTENT);   /// 800
    lv_obj_set_height(ui_compass, LV_SIZE_CONTENT);    /// 800
    lv_obj_set_align(ui_compass, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_compass, LV_OBJ_FLAG_CLICKABLE);     /// Flags
    lv_obj_remove_flag(ui_compass, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_compass, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(ui_compass, 0, 0);

    ui_heading_bug = lv_image_create(ui_main_scr);
    lv_image_set_src(ui_heading_bug, &heading_bug);
    lv_obj_set_width(ui_heading_bug, LV_SIZE_CONTENT);   /// 56
    lv_obj_set_height(ui_heading_bug, LV_SIZE_CONTENT);    /// 36
    lv_obj_set_align(ui_heading_bug, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_heading_bug, LV_OBJ_FLAG_CLICKABLE);     // Flags
    lv_obj_remove_flag(ui_heading_bug, LV_OBJ_FLAG_SCROLLABLE);  // Flags    
}

void ui_zoom_control_init(void)
{
   ui_zoom = lv_arc_create(ui_main_scr);
    lv_obj_set_width(ui_zoom, 655);
    lv_obj_set_height(ui_zoom, 655);
    lv_obj_set_align(ui_zoom, LV_ALIGN_CENTER);
    lv_arc_set_bg_angles(ui_zoom, 145, 225);
    lv_arc_set_range(ui_zoom, 10, 18);
    lv_arc_set_value(ui_zoom, 12);
    
    lv_obj_set_style_arc_color(ui_zoom, lv_color_hex(0x4040FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_zoom, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(ui_zoom, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_arc_color(ui_zoom, lv_color_hex(0xFCAF00), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_zoom, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(ui_zoom, 5, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(ui_zoom, 5, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_zoom, lv_color_hex(0xFCAF00), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_zoom, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_zoom, lv_color_hex(0xFCAF00), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(ui_zoom, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_zoom, 4, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_pad(ui_zoom, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    /* Add the event callback function */
    lv_obj_add_event_cb(ui_zoom, zoom_event_cb, LV_EVENT_ALL, NULL); // Using LV_EVENT_ALL to catch both
}

void ui_upper_txt_init(void)
{
    // Conceptual example for arc label
    ui_upper_txt = lv_arclabel_create(ui_main_scr);
    lv_obj_set_width(ui_upper_txt, 675);
    lv_obj_set_height(ui_upper_txt, 675);
    lv_obj_center(ui_upper_txt);
    lv_arclabel_set_text(ui_upper_txt, "Curved Text Example");
    lv_arclabel_set_radius(ui_upper_txt, 675);
    lv_arclabel_set_angle_start(ui_upper_txt, 150);        // Start angle (degrees)
    lv_arclabel_set_angle_size(ui_upper_txt, 240);         // Angular span
    lv_arclabel_set_dir(ui_upper_txt, LV_ARCLABEL_DIR_CLOCKWISE);
    lv_arclabel_set_text_horizontal_align(ui_upper_txt, LV_ARCLABEL_TEXT_ALIGN_CENTER);
    lv_arclabel_set_text_vertical_align(ui_upper_txt, LV_ARCLABEL_TEXT_ALIGN_CENTER);

    lv_obj_set_style_text_color(ui_upper_txt, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui_upper_txt, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
        
    lv_obj_set_style_text_opa(ui_upper_txt, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_upper_txt, &lv_font_montserrat_38, LV_PART_MAIN | LV_STATE_DEFAULT);
}
void ui_mainscreen_screen_destroy(void)
{
    if(ui_main_scr) lv_obj_del(ui_main_scr);

    // NULL screen variables
    ui_main_scr = NULL;
    ui_compass = NULL;
    ui_heading_bug = NULL;
    ui_Container1 = NULL;
    ui_lbllatitude = NULL;
    ui_lbllongitude = NULL;
    ui_lblcourse = NULL;
    ui_Container2 = NULL;
    ui_lbllatitudeval = NULL;
    ui_lbllongitudeval = NULL;
    ui_lblcourseval = NULL;
}

/*TODO: current location dot to arrow when moving text on curve */
void ui_init(void)
{
    ui_mainscreen_screen_init();
    GPSMap_init(ui_main_scr);
   // gps_data();
   ui_zoom_control_init();
   ui_upper_txt_init();
    ui____initial_actions0 = lv_obj_create(NULL);
    lv_disp_load_scr(ui_main_scr);

}

void ui_destroy(void)
{
    ui_mainscreen_screen_destroy();
}

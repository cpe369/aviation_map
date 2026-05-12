

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "nmea_parser.h"

//#define MAP_TEST    0
///////////////////// SCREENS ////////////////////

//#include "screens/ui_Screen1.h"

///////////////////// VARIABLES ////////////////////


// EVENTS

//extern lv_obj_t * ui____initial_actions0;

// IMAGES AND IMAGE SETS
LV_IMG_DECLARE(compass);
LV_IMG_DECLARE(heading_bug);



// UI INIT
void ui_init(void);
void ui_destroy(void);

void update_values(gps_t gps_data);
void animate_heading_bug(float heading);


#ifdef __cplusplus
} /*extern "C"*/
#endif



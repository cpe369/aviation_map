#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nmea_parser.h"
#include "nmea_driver.h"
#include "math.h"

extern "C" {
    nmea_parser_handle_t nmea_parser_init(const nmea_parser_config_t *config);
    esp_err_t nmea_parser_deinit(nmea_parser_handle_t nmea_hdl);
    esp_err_t nmea_parser_add_handler(nmea_parser_handle_t nmea_hdl, esp_event_handler_t event_handler, void *handler_args);
    esp_err_t nmea_parser_remove_handler(nmea_parser_handle_t nmea_hdl, esp_event_handler_t event_handler);
}

static const char *TAG = "gps_data";
static nmea_parser_handle_t nmea_hdl = NULL;

#define TIME_ZONE (+8)   //Beijing Time
#define YEAR_BASE (2000) //date in GPS starts from 2000

gps_t gps_info;

gps_t get_gps_info() {
    return gps_info;
}   

void update_gps_info(gps_t *gps) {
    

    memcpy(&gps_info, gps, sizeof(gps_t));
    

    //printf("Received GPS Data - Latitude: %f, Longitude: %f\n", gps->latitude, gps->longitude);
}

/**
 * @brief GPS Event Handler
 *
 * @param event_handler_arg handler specific arguments
 * @param event_base event base, here is fixed to ESP_NMEA_EVENT
 * @param event_id event id
 * @param event_data event specific arguments
 */
static void gps_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    gps_t *gps = NULL;
    switch (event_id) {
    case GPS_UPDATE:
        gps = (gps_t *)event_data;
        /* print information parsed from GPS statements */
        // ESP_LOGI(TAG, "===== GPS DATA UPDATE =====\r\n"
        //          "Date/Time: %d/%d/%d %d:%d:%d.%d\r\n"
        //          "Latitude:  %.05f° %s\r\n"
        //          "Longitude: %.05f° %s\r\n"
        //          "Altitude:  %.02f m\r\n"
        //          "Speed:     %.2f m/s\r\n"
        //          "COG:       %.02f°\r\n"
        //          "Variation: %.02f°\r\n"
        //          "Valid:     %s\r\n"
        //          "Fix:       %d (0=Invalid, 1=GPS, 2=DGPS, 3=PPS, 4=RTK, 5=FloatRTK, 6=Estimated, 7=Manual, 8=Simulation)\r\n"
        //          "Fix Mode:  %d (1=NoFix, 2=2D, 3=3D)\r\n"
        //          "Sats Used: %d\r\n"
        //          "Sats View: %d\r\n"
        //          "HDOP:      %.2f\r\n"
        //          "VDOP:      %.2f\r\n"
        //          "PDOP:      %.2f",
        //          gps->date.year + YEAR_BASE, gps->date.month, gps->date.day,
        //          gps->tim.hour + TIME_ZONE, gps->tim.minute, gps->tim.second, gps->tim.thousand,
        //          fabsf(gps->latitude), gps->latitude >= 0 ? "N" : "S",
        //          fabsf(gps->longitude), gps->longitude >= 0 ? "E" : "W",
        //          gps->altitude, gps->speed, gps->cog, gps->variation,
        //          gps->valid ? "YES" : "NO",
        //          gps->fix, gps->fix_mode, gps->sats_in_use, gps->sats_in_view,
        //          gps->dop_h, gps->dop_v, gps->dop_p);

        update_gps_info(gps);
        break;
    case GPS_UNKNOWN:
        /* print unknown statements */
        ESP_LOGW(TAG, "Unknown statement:%s", (char *)event_data);
        break;
    default:
        break;
    }
}

void gps_init(void)
{
    memset(&gps_info, 0, sizeof(gps_t));
    /* NMEA parser configuration */
    nmea_parser_config_t config = NMEA_PARSER_CONFIG_DEFAULT();
    /* init NMEA parser library */
    nmea_hdl = nmea_parser_init(&config);

    /* register event handler for NMEA parser library */
    nmea_parser_add_handler(nmea_hdl, gps_event_handler, NULL);
    
}

void gps_deinit(void)
{
    if (nmea_hdl != NULL)
    {
        /* unregister event handler */
        nmea_parser_remove_handler(nmea_hdl, gps_event_handler);
        /* deinit NMEA parser library */
        nmea_parser_deinit(nmea_hdl);
    }
}
#pragma once


#ifdef __cplusplus
extern "C" {
#endif
#include "nmea_parser.h"

void gps_init(void);
void gps_deinit(void);
gps_t get_gps_info(void);

#ifdef __cplusplus
}
#endif
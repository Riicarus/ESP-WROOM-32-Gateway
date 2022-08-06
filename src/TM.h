#ifndef TM_H
#define TM_H

#include <string>
#include <WiFi.h>

const std::string ntp_server = "pool.ntp.org";
const long gmt_offset_sec = 8 * 3600;
const int daylight_offset_sec = 8 * 3600;

void printLocalTime() {
    struct tm time_info;
    if (!getLocalTime(&time_info)) {
        Serial.println("Failed to obtain time");
        return;
    }

    Serial.println(&time_info, "%F %T %A");
}

/* 
    get time from ntp server,
    the chip will use RTC clock to keep time right if succeeded.
 */
void time_init() {
    configTime(gmt_offset_sec, daylight_offset_sec, ntp_server.c_str());
}


#endif
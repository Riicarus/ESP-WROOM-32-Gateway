#ifndef OLED_H
#define OLED_H

#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define OLED_CLK 18
#define OLED_MOSI 19
#define OLED_RES 16
#define OLED_DC 17
#define OLED_CS 5

#define OLED_ADDRESS 0x3C

U8G2_SH1106_128X64_NONAME_1_4W_SW_SPI u8g2(U8G2_R0, OLED_CLK, OLED_MOSI, OLED_CS, OLED_DC, OLED_RES);

std::string TH_cache = "";
std::string light_cache = "";
std::string notice_cache = "";
std::string alive_device_number_cache = "";

void oled_setup()
{
    u8g2.begin();
    u8g2.enableUTF8Print();
}

void oled_draw_whole_screen(const char *chars) {
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.firstPage();
    do
    {
        u8g2.setCursor(0, 35);
        u8g2.print(chars);
    } while (u8g2.nextPage());
}

void draw_with_cache()
{
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.firstPage();
    do
    {
        u8g2.setCursor(0, 10);
        u8g2.print(TH_cache.c_str());

        u8g2.setCursor(80, 10);
        u8g2.print(light_cache.c_str());

        u8g2.setCursor(0, 20);
        std::string alive_deivce_number_out = "alive: " + alive_device_number_cache;
        u8g2.print(alive_deivce_number_out.c_str());

        u8g2.setCursor(0, 60);
        u8g2.print(notice_cache.c_str());
    } while (u8g2.nextPage());
}

void oled_update_TH(const char *chars)
{
    TH_cache = chars;
    draw_with_cache();
}

void oled_update_light(const char *chars)
{
    light_cache = chars;
    draw_with_cache();
}

void oled_update_notice(const char *chars)
{
    notice_cache = chars;
    draw_with_cache();
}

void oled_update_alive_device_number(const char *chars)
{
    alive_device_number_cache = chars;
    draw_with_cache();
}

#endif
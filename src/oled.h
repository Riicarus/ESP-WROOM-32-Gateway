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

std::string alive_device_number_cache = "";

std::string notice_cache = "";

std::string device_aliasName_cache = "";
std::string property_part_cache_1 = "";
std::string property_part_cache_2 = "";
std::string property_part_cache_3 = "";

void oled_setup()
{
    u8g2.begin();
    u8g2.enableUTF8Print();
}

void oled_draw_whole_screen(const char *chars)
{
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
        u8g2.print(device_aliasName_cache.c_str());

        u8g2.setCursor(0, 20);
        u8g2.print(property_part_cache_1.c_str());

        u8g2.setCursor(0, 30);
        u8g2.print(property_part_cache_2.c_str());

        u8g2.setCursor(0, 40);
        u8g2.print(property_part_cache_3.c_str());

        u8g2.setCursor(0, 52);
        u8g2.print(alive_device_number_cache.c_str());

        u8g2.setCursor(0, 62);
        u8g2.print(notice_cache.c_str());
    } while (u8g2.nextPage());
}

void oled_update_device_aliasName_cache(const char *aliasName)
{
    device_aliasName_cache = aliasName;

    draw_with_cache();
}

void oled_update_property_cache(const char *property)
{
    property_part_cache_1 = property_part_cache_2;
    property_part_cache_2 = property_part_cache_3;
    property_part_cache_3 = property;

    draw_with_cache();
}

void oled_clean_property_cache()
{
    property_part_cache_1 = "";
    property_part_cache_2 = "";
    property_part_cache_3 = "";

    draw_with_cache();
}

void oled_update_alive_device_number(const char *chars)
{
    alive_device_number_cache = chars;
    draw_with_cache();
}

void oled_update_notice(const char *chars)
{
    notice_cache = chars;
    draw_with_cache();
}

#endif
#include <Arduino.h>
#include <Tasks.h>
#include <oled.h>

void setup()
{
    Serial.begin(115200);

    oled_update_notice("Device Start");

    BLE_app_main();
    OLED_app_main();
    network_info_report_app_main();
    Mqtt_app_main();
    device_alive_check_app_main();

}

void loop()
{
    delay(1000);
}
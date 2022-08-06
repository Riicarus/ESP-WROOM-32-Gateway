#include <Arduino.h>
#include <Tasks.h>

void setup()
{
    Serial.begin(115200);

    BLE_app_main();
    network_info_report_app_main();
    Mqtt_app_main();
    device_alive_check_app_main();

}

void loop()
{
    delay(1000);
}
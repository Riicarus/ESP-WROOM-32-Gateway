#ifndef TASKS_H
#define TASKS_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <BLE_Tool.h>
#include <DataUploader.h>
#include <Gateway.h>
#include <MqttCallBacks.h>
#include <WiFi_Tool.h>
#include <map>
#include <oled.h>
#include <server_info_getter.h>
#include <vector>

// ------------------------ Network info report Task ------------------------//
TaskHandle_t network_info_report_task_handle;

esp_timer_handle_t network_info_report_timer = 0;

void network_info_report_timer_periodic(void *arg);

void network_info_report_task(void *network_info_report_task)
{

    while (1)
    {
        if (!isWiFiConnected() || !mqtt_client.connected())
        {
            Serial.println("No WiFi or Mqtt connection, reporting network info failed!");
            Serial.println("Please check WiFi or Mqtt connection!");
        }
        else
        {
            network_info_report();
        }

        vTaskSuspend(NULL);
    }
}

void network_info_report_app_main(void)
{

    esp_timer_create_args_t start_network_info_report = {
        .callback = &network_info_report_timer_periodic, .arg = NULL, .name = "NetworkInfoReportPeriodicTimer"};

    esp_timer_init();

    esp_timer_create(&start_network_info_report, &network_info_report_timer);

    esp_timer_start_periodic(network_info_report_timer, 60 * 1000 * 1000);

    xTaskCreatePinnedToCore(network_info_report_task, "network_info_report_task", 6000, NULL, 1,
                            &network_info_report_task_handle, 0);
}

void network_info_report_timer_periodic(void *arg)
{
    vTaskResume(network_info_report_task_handle);
}

// ------------------------ Mqtt Task ------------------------//
TaskHandle_t Mqtt_task_handle;

esp_timer_handle_t Mqtt_timer = 0;

void Mqtt_timer_periodic(void *arg);

void Mqtt_task(void *Mqtt_task)
{
    std::string broker = "124.222.158.8";
    std::string username = "mosquitto";
    std::string password = "314159.com";
    int port = 1883;

    while (1)
    {
        // connect to mqtt server
        if (isWiFiConnected() && !isMqttConnected())
        {
            mqtt_init(broker, port, username, password, customizedMqttCallback);

            // if connected, subscribe topic
            if (isMqttConnected())
            {
                mqtt_subscribe(network_info_report_topic);
            }
        }
        // start mqtt receive service
        mqtt_client.loop();

        vTaskSuspend(NULL);
    }
}

void Mqtt_app_main(void)
{
    esp_timer_create_args_t start_Mqtt = {.callback = &Mqtt_timer_periodic, .arg = NULL, .name = "MqttPeriodicTimer"};

    esp_timer_init();

    esp_timer_create(&start_Mqtt, &Mqtt_timer);

    esp_timer_start_periodic(Mqtt_timer, 1000 * 1000);

    xTaskCreatePinnedToCore(Mqtt_task, "Mqtt_task", 6000, NULL, 1, &Mqtt_task_handle, 0);
}

void Mqtt_timer_periodic(void *arg)
{
    vTaskResume(Mqtt_task_handle);
}

// ------------------------ BLE Task ------------------------//
TaskHandle_t BLE_task_handle;

esp_timer_handle_t BLE_timer = 0;

void BLE_timer_periodic(void *arg);

void BLE_task(void *BLE_task)
{
    start_BLE();

    while (1)
    {
        // keep BLE alive
        handle_BLE_service();

        vTaskSuspend(NULL);
    }
}

void BLE_app_main(void)
{
    esp_timer_create_args_t start_BLE = {.callback = &BLE_timer_periodic, .arg = NULL, .name = "BLEPeriodicTimer"};

    esp_timer_init();

    esp_timer_create(&start_BLE, &BLE_timer);

    esp_timer_start_periodic(BLE_timer, 1000 * 1000);

    xTaskCreatePinnedToCore(BLE_task, "BLE_task", 4000, NULL, 1, &BLE_task_handle, 1);
}

void BLE_timer_periodic(void *arg)
{
    vTaskResume(BLE_task_handle);
}

// ------------------------ Device Alive Check Task ------------------------//
TaskHandle_t device_alive_check_task_handle;

esp_timer_handle_t device_alive_check_timer = 0;

void device_alive_check_timer_periodic(void *arg);

void device_alive_check_task(void *device_alive_check_task)
{
    while (1)
    {
        device_alive_check();

        if (isWiFiConnected())
        {
            int number = getAliveDeviceNumber();
            UPLOAD_DATA datas[1];
            datas[0].key = "alive-device-number";
            datas[0].value = std::to_string(number);
            uploadData(datas, 1);
        }

        vTaskSuspend(NULL);
    }
}

void device_alive_check_app_main(void)
{
    esp_timer_create_args_t start_device_alive_check = {
        .callback = &device_alive_check_timer_periodic, .arg = NULL, .name = "device_alive_checkPeriodicTimer"};

    esp_timer_init();

    esp_timer_create(&start_device_alive_check, &device_alive_check_timer);

    esp_timer_start_periodic(device_alive_check_timer, 60 * 1000 * 1000);

    xTaskCreatePinnedToCore(device_alive_check_task, "device_alive_check_task", 4000, NULL, 1,
                            &device_alive_check_task_handle, 1);
}

void device_alive_check_timer_periodic(void *arg)
{
    vTaskResume(device_alive_check_task_handle);
}

// ------------------------ OLED Task ------------------------//
TaskHandle_t OLED_task_handle;

esp_timer_handle_t OLED_timer = 0;

void OLED_timer_periodic(void *arg);

void OLED_task(void *OLED_task)
{
    oled_setup();

    std::vector<SENSOR_INFO> sensor_infos;

    bool show_complete = true;

    while (1)
    {
        if (!isWiFiConnected())
        {
            oled_draw_whole_screen("NO WiFi ...");
            vTaskSuspend(NULL);
        }
        else
        {
            if (show_complete)
            {
                sensor_infos = get_sensor_data();

                if (!sensor_infos.empty())
                {
                    show_complete = false;
                }
                else {
                    // update alive_device_number here to avoid being blocked too long
                    int alive_device_number = getAliveDeviceNumber();
                    std::string alive_device_number_msg = "alive: " + std::to_string(alive_device_number);
                    oled_update_alive_device_number(alive_device_number_msg.c_str());

                    vTaskSuspend(NULL);
                }
            }
            else
            {
                std::vector<SENSOR_INFO>::iterator sensor_info_itr = sensor_infos.begin();
                while (sensor_info_itr != sensor_infos.end())
                {
                    oled_update_device_aliasName_cache((*sensor_info_itr).aliasName.c_str());
                    std::map<std::string, std::string>::iterator property_itr = (*sensor_info_itr).properties.begin();
                    while (property_itr != (*sensor_info_itr).properties.end())
                    {

                        std::string msg = property_itr->first + ": " + property_itr->second;
                        oled_update_property_cache(msg.c_str());

                        // update alive_device_number here to avoid being blocked too long
                        int alive_device_number = getAliveDeviceNumber();
                        std::string alive_device_number_msg = "alive: " + std::to_string(alive_device_number);
                        oled_update_alive_device_number(alive_device_number_msg.c_str());

                        vTaskSuspend(NULL);

                        property_itr++;
                    }

                    oled_clean_property_cache();

                    sensor_info_itr++;
                }
                show_complete = true;
            }
        }
    }
}

void OLED_app_main(void)
{
    esp_timer_create_args_t start_OLED = {.callback = &OLED_timer_periodic, .arg = NULL, .name = "OLEDPeriodicTimer"};

    esp_timer_init();

    esp_timer_create(&start_OLED, &OLED_timer);

    esp_timer_start_periodic(OLED_timer, 3000 * 1000);

    xTaskCreatePinnedToCore(OLED_task, "OLED_task", 10000, NULL, 1, &OLED_task_handle, 1);
}

void OLED_timer_periodic(void *arg)
{
    vTaskResume(OLED_task_handle);
}

#endif
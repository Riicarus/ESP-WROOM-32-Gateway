#ifndef TASKS_H
#define TASKS_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <BLE_Tool.h>
#include <DHT11_Sensor.h>
#include <DataUploader.h>
#include <Gateway.h>
#include <Light_Sensor.h>
#include <MqttCallBacks.h>
#include <WiFi_Tool.h>

// ------------------------ DHT11 Sensor Task ------------------------//

TaskHandle_t dht_task_handle;

esp_timer_handle_t DHT11_temp_humi_timer = 0;

void DHT11_timer_periodic(void *arg);

void DHT11_task(void *DHT11_task)
{
    gpio_pad_select_gpio(DHT11_PIN);

    while (1)
    {
        DHT11_Start();
        if (DHT11_ReadTemHum(DHT11Data))
        {
            Temp = DHT11Data[2];
            Humi = DHT11Data[0];

            printf("Temp= %d, Humi= %d\r\n", Temp, Humi);

            if (isWiFiConnected())
            {
                UPLOAD_DATA datas[2];
                datas[0].key = "temperature";
                datas[0].value = std::to_string(Temp);

                datas[1].key = "humidity";
                datas[1].value = std::to_string(Humi);

                uploadData(datas, 2);
            }
        }
        else
        {
            printf("DHT11 Error!\r\n");
        }

        // 这里是把自己挂起，挂起后该任务被暂停，不恢复是不运行的
        vTaskSuspend(NULL);
    }
}

void DHT11_app_main(void)
{
    // 配置定时器
    esp_timer_create_args_t start_dht = {.callback = &DHT11_timer_periodic, .arg = NULL, .name = "DHT11PeriodicTimer"};
    // 定时器初始化
    esp_timer_init();
    // 创建定时器
    esp_timer_create(&start_dht, &DHT11_temp_humi_timer);
    // 定时器每 1min 中断一次
    esp_timer_start_periodic(DHT11_temp_humi_timer, 60 * 1000 * 1000);

    //创建任务，并且使用一个核心，毕竟ESP32是双核，0是第一个核心，1是第二个。
    //任务栈的大小由自己来测试，代码这里有空余多的空间。
    //任务级别尽量高点，这里为2；再放入任务句柄，以便用于定时中断来恢复任务
    xTaskCreatePinnedToCore(DHT11_task, "DHT11_task", 4000, NULL, 2, &dht_task_handle, 1);
}

//定时器的回调函数
void DHT11_timer_periodic(void *arg)
{
    vTaskResume(dht_task_handle); //恢复任务
}

// ------------------------ Light Sensor Task ------------------------//
TaskHandle_t light_sensor_task_handle;

esp_timer_handle_t light_sensor_timer = 0;

void light_sensor_timer_periodic(void *arg);

void light_sensor_task(void *light_sensor_task)
{
    light_sensor_start();

    long timer = 0;

    while (1)
    {
        int light_sensor_data = readLightSensorData();

        Serial.printf("Light: %d\r\n", light_sensor_data);

        if ((timer >= 10) && (timer % 10 == 0) && isWiFiConnected())
        {
            UPLOAD_DATA datas[1];
            datas[0].key = "light";
            datas[0].value = std::to_string(light_sensor_data);
            uploadData(datas, 1);
        }

        timer++;
        vTaskSuspend(NULL);
    }
}

void light_sensor_app_main(void)
{
    esp_timer_create_args_t start_light_sensor = {
        .callback = &light_sensor_timer_periodic, .arg = NULL, .name = "LightSensorPeriodicTimer"};

    esp_timer_init();

    esp_timer_create(&start_light_sensor, &light_sensor_timer);

    esp_timer_start_periodic(light_sensor_timer, 3000 * 1000);

    xTaskCreatePinnedToCore(light_sensor_task, "light_sensor_task", 4000, NULL, 2, &light_sensor_task_handle, 1);
}

void light_sensor_timer_periodic(void *arg)
{
    vTaskResume(light_sensor_task_handle);
}

// ------------------------ WiFi info report Task ------------------------//
TaskHandle_t WiFi_infor_report_task_handle;

esp_timer_handle_t WiFi_infor_report_timer = 0;

void WiFi_infor_report_timer_periodic(void *arg);

void WiFi_infor_report_task(void *WiFi_infor_report_task)
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
            WiFi_info_report();
        }

        vTaskSuspend(NULL);
    }
}

void WiFi_info_report_app_main(void)
{

    esp_timer_create_args_t start_WiFi_infor_report = {
        .callback = &WiFi_infor_report_timer_periodic, .arg = NULL, .name = "WiFiInfoReportPeriodicTimer"};

    esp_timer_init();

    esp_timer_create(&start_WiFi_infor_report, &WiFi_infor_report_timer);

    esp_timer_start_periodic(WiFi_infor_report_timer, 60 * 1000 * 1000);

    xTaskCreatePinnedToCore(WiFi_infor_report_task, "WiFi_infor_report_task", 6000, NULL, 1,
                            &WiFi_infor_report_task_handle, 0);
}

void WiFi_infor_report_timer_periodic(void *arg)
{
    vTaskResume(WiFi_infor_report_task_handle);
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
                mqtt_subscribe(WiFi_report_topic);
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

#endif
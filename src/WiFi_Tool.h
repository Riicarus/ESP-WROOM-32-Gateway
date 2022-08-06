#ifndef WIFI_H
#define WIFI_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Device_Info.h>
#include <Mqtt_Tool.h>
#include <TM.h>
#include <WiFi.h>

const std::string network_info_report_topic = "/device/network/report";

/*
    report network information of json type to mqtt server
 */
void network_info_report()
{
    DynamicJsonDocument doc(1024);
    doc["ip"] = WiFi.localIP();
    doc["mac"] = WiFi.macAddress();
    doc["deviceId"] = deviceId;

    std::string output;
    serializeJson(doc, output);

    mqtt_publish(network_info_report_topic, output);
}

/*
    connnect WiFi, init time, report to gateway if needed
 */
void WiFi_Connect(std::string name, std::string password, bool report)
{
    Serial.print("Connecting ..");
    WiFi.begin(name.c_str(), password.c_str());

    int timer = 0;
    while ((WiFi.status() != WL_CONNECTED) && (timer <= 5000))
    {
        //这里是阻塞程序，直到连接成功
        delay(500);
        timer += 300;
        Serial.print(".");
    }
    Serial.println();

    if (timer >= 5000)
    {
        Serial.println("Connection Failed, Time out. Please try to chech WiFi info.");
        return;
    }

    Serial.println("WiFi Connected!");
    Serial.printf("IP Address: %s\r\n", WiFi.localIP().toString().c_str());
    Serial.printf("Mac Address: %s\r\n", WiFi.macAddress().c_str());

    // get time when WiFi connected
    time_init();

    int tried_count = 0;
    while (report && tried_count <= 5)
    {
        if (mqtt_client.connected())
        {
            network_info_report();
            return;
        }
        delay(2000);

        tried_count++;
    }
}

bool isWiFiConnected()
{
    return WiFi.isConnected();
}

#endif
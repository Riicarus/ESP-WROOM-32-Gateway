#ifndef MQTT_CALLBACKS_H
#define MQTT_CALLBACKS_H

#include <Gateway.h>
#include <WiFi_Tool.h>

void defaltCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.println();
    Serial.print("Mqtt message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Mqtt message received:");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
    Serial.println();
}

// report device info to server if gateway received network info report
void network_info_report_callback(char *topic, byte *payload, unsigned int length)
{
    // todo judge if it is network info report. if not, return.
    std::string topic_str = topic;
    if (network_info_report_topic.compare(topic_str) != 0)
    {
        return;
    }

    time_t now;
    time(&now);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    device_heart_handler(doc["deviceId"]);

    HTTPClient http_client;
    http_client.begin(network_info_report_dst_url.c_str());
    http_client.addHeader("Content-Type", "application/json");

    doc["registerTime"] = now;

    std::string output;
    serializeJson(doc, output);

    int http_code = http_client.POST(output.c_str());

    if (http_code > 0)
    {
        Serial.printf("HTTP POST Result Code: %d\r\n", http_code);

        // receive right containts
        if (http_code == HTTP_CODE_OK)
        {
            String resBuff = http_client.getString();

            Serial.println("HTTP Received One Message: ");
            Serial.println(resBuff);

            deserializeJson(doc, resBuff);

            bool isAllowed = doc["data"];
            if (isAllowed)
            {
                Serial.println("Device Registration Succeeded!");
            }
            else
            {
                Serial.println("Device Registration Failed!");
            }
        }
    }
    else
    {
        Serial.printf("HTTP POST Error: %s\r\n", http_client.errorToString(http_code).c_str());
    }
}

void customizedMqttCallback(char *topic, byte *payload, unsigned int length)
{
    defaltCallback(topic, payload, length);
    network_info_report_callback(topic, payload, length);
}

#endif
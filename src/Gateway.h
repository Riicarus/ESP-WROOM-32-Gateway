#ifndef GATEWAY_H
#define GATEWAY_H

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <TM.h>
#include <map>
#include <string>

const std::string network_info_report_dst_url = "http://192.168.43.18:9000/device/register";
const std::string unregister_report_dst_url = "http://192.168.43.18:9000/device/unregister";
const int expireTime = 70000;
std::map<std::string, long> device_heart_map;

// maintain device_heart_map, update device last heart time
void device_heart_handler(std::string deviceId)
{
    time_t now;
    time(&now);

    std::map<std::string, long>::iterator itr;
    itr = device_heart_map.find(deviceId);

    if (itr != device_heart_map.end())
    {
        itr->second = now;
        Serial.printf("Update device[%s] heart break\r\n", deviceId.c_str());
    }
    else
    {
        device_heart_map.insert(std::pair<std::string, long>(deviceId, now));
        Serial.printf("Newly received one device[%s] heart break, registered.\r\n", deviceId.c_str());
    }
}

void device_alive_check()
{
    time_t now;
    time(&now);

    Serial.println("Start check device heart break ...");
    std::map<std::string, long>::iterator itr;

    // traverse all device
    for (itr = device_heart_map.begin(); itr != device_heart_map.end(); itr++)
    {
        long time_interval = now - itr->second;
        if (time_interval >= expireTime)
        {
            // heart break time expire, report to server
            DynamicJsonDocument doc(1024);
            doc["deviceId"] = itr->first;
            doc["unregisterTime"] = now;
            doc["authtoken"] = "authtoken";

            std::string output;
            serializeJson(doc, output);

            HTTPClient http_client;
            http_client.begin(unregister_report_dst_url.c_str());
            http_client.addHeader("Content-Type", "application/json");

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

                    bool succeeded = doc["data"];
                    if (succeeded)
                    {
                        Serial.printf("Device[%s] unregistered from server.", deviceId.c_str());
                    }
                    else
                    {
                        Serial.printf("Device[%s] unregistered failed.", deviceId.c_str());
                    }
                }
            }
            else
            {
                Serial.printf("HTTP POST Error: %s\r\n", http_client.errorToString(http_code).c_str());
            }
        }
    }

    Serial.println("Device heart break check ends");
}

int getAliveDeviceNumber()
{
    std::map<std::string, long>::iterator itr = device_heart_map.begin();

    int count = 0;
    while (itr != device_heart_map.end())
    {
        count++;
        itr++;
    }

    return count;
}

#endif
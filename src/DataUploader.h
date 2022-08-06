#ifndef DATA_UPLOADER_H
#define DATA_UPLOADER_H

#include <string>
#include <ArduinoJson.h>
#include <Device_Info.h>
#include <HTTPClient.h>

typedef struct upload_data
{
    std::string key;
    std::string value;
} UPLOAD_DATA;

const std::string data_upload_url = "http://192.168.43.18:9000/device/report";

void uploadData(UPLOAD_DATA *upload_data, int length) {
    DynamicJsonDocument doc(1024);

    doc["deviceId"] = deviceId;
    JsonObject dataMap = doc.createNestedObject("dataMap");

    for (int i = 0; i < length; i++)
    {
        UPLOAD_DATA data = upload_data[i];
        dataMap[data.key] = data.value;
    }

    std::string output;
    serializeJson(doc, output);

    Serial.printf("Device[%s] report data: \r\n", deviceId.c_str());
    Serial.println(output.c_str());

    HTTPClient http_client;
    http_client.begin(data_upload_url.c_str());
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
                Serial.printf("Device[%s] report data to server.", deviceId.c_str());
            }
            else
            {
                Serial.printf("Device[%s] report data failed.", deviceId.c_str());
            }
        }
    }
    else
    {
        Serial.printf("HTTP POST Error: %s\r\n", http_client.errorToString(http_code).c_str());
    }
}

#endif
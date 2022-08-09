#ifndef SERVER_INFO_GETTER_H
#define SERVER_INFO_GETTER_H

#include <ArduinoJson.h>
#include <Device_Info.h>
#include <http_service.h>

std::string get_sensor_data_url = "http://192.168.43.18:9000/device/" + deviceId + "/sensor/data";

std::string get_server_sensor_data()
{
    DynamicJsonDocument doc(1024);

    std::string res = http_get(get_sensor_data_url.c_str());
    deserializeJson(doc, res);

    return doc["data"].as<std::string>();
}

#endif
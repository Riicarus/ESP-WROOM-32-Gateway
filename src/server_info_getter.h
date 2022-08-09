#ifndef SERVER_INFO_GETTER_H
#define SERVER_INFO_GETTER_H

#include <ArduinoJson.h>
#include <Device_Info.h>
#include <http_service.h>
#include <map>
#include <string_tool.h>
#include <vector>

typedef struct sensor_info
{
    std::string deviceId;
    std::string aliasName;
    std::map<std::string, std::string> properties;

} SENSOR_INFO;

std::string get_sensor_data_url = "http://192.168.43.18:9000/device/" + deviceId + "/sensor/data";
std::string get_sensor_info_url = "http://192.168.43.18:9000/device/" + deviceId + "/sensor/info";

std::string get_server_sensor_data()
{
    DynamicJsonDocument doc(1024);

    std::string res = http_get(get_sensor_data_url.c_str());
    deserializeJson(doc, res);

    return doc["data"].as<std::string>();
}

std::string get_sensor_info_data()
{
    DynamicJsonDocument doc(1024);

    std::string res = http_get(get_sensor_info_url.c_str());
    deserializeJson(doc, res);

    return doc["data"].as<std::string>();
}

std::vector<SENSOR_INFO> get_sensor_data()
{
    std::vector<SENSOR_INFO> sensor_infos;

    std::string sensor_info_json = get_sensor_info_data();

    if (sensor_info_json == "[]")
    {
        return sensor_infos;
    }

    sensor_info_json = sensor_info_json.substr(2, sensor_info_json.length() - 4);

    std::vector<std::string> sensor_info_json_list = string_split(sensor_info_json, ",");
    std::vector<std::string>::iterator itr;
    itr = sensor_info_json_list.begin();

    while (itr != sensor_info_json_list.end())
    {
        std::vector<std::string> parts = string_split((*itr), "*");
        SENSOR_INFO sensor_info;
        sensor_info.deviceId = parts[0];
        sensor_info.aliasName = parts[1];

        std::string propertyIds_json = parts[2];
        std::vector<std::string> propertyIds = string_split(propertyIds_json, "/");

        std::vector<std::string>::iterator propertyIds_itr;
        propertyIds_itr = propertyIds.begin();
        while (propertyIds_itr != propertyIds.end())
        {
            sensor_info.properties.insert(std::pair<std::string, std::string>((*propertyIds_itr), "n"));
            propertyIds_itr++;
        }

        sensor_infos.push_back(sensor_info);

        itr++;
    }

    std::string sensor_data_json = get_server_sensor_data();

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, sensor_data_json);

    std::vector<SENSOR_INFO>::iterator sensor_info_itr;
    sensor_info_itr = sensor_infos.begin();
    while (sensor_info_itr != sensor_infos.end())
    {
        std::map<std::string, std::string>::iterator map_itr;

        map_itr = (*sensor_info_itr).properties.begin();

        while (map_itr != (*sensor_info_itr).properties.end())
        {
            map_itr->second = doc[(*sensor_info_itr).deviceId.c_str()][map_itr->first.c_str()].as<std::string>();

            map_itr++;
        }

        sensor_info_itr++;
    }

    return sensor_infos;
}

#endif
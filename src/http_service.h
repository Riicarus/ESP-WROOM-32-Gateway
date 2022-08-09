#ifndef HTTP_SERVICE_H
#define HTTP_SERVICE_H

#include <HTTPClient.h>
#include <string>

HTTPClient http_client;

std::string http_get(const char *url)
{
    http_client.begin(url);
    http_client.addHeader("Content-Type", "application/json");

    int http_code = http_client.GET();

    if (http_code > 0)
    {
        Serial.printf("HTTP GET Result Code: %d\r\n", http_code);

        // receive right containts
        if (http_code == HTTP_CODE_OK)
        {
            String resBuff = http_client.getString();

            Serial.println("HTTP Received One Message: ");
            Serial.println(resBuff);

            return resBuff.c_str();
        }
    }
    else
    {
        Serial.printf("HTTP GET Error: %s\r\n", http_client.errorToString(http_code).c_str());

        return "";
    }

    return "";
}

std::string http_post(const char *url, const char *data)
{
    http_client.begin(url);
    http_client.addHeader("Content-Type", "application/json");

    int http_code = http_client.POST(data);

    if (http_code > 0)
    {
        Serial.printf("HTTP POST Result Code: %d\r\n", http_code);

        // receive right containts
        if (http_code == HTTP_CODE_OK)
        {
            String resBuff = http_client.getString();

            Serial.println("HTTP Received One Message: ");
            Serial.println(resBuff);

            return resBuff.c_str();
        }
    }
    else
    {
        Serial.printf("HTTP POST Error: %s\r\n", http_client.errorToString(http_code).c_str());

        return "";
    }

    return "";
}

#endif
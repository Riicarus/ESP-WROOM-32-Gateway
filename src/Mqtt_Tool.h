#ifndef MQTT_TOOL_H
#define MQTT_TOOL_H

#include <PubSubClient.h>
#include <WiFi.h>
#include <string>

std::string mqtt_broker;
std::string mqtt_username;
std::string mqtt_password;
int mqtt_port = 1883;

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);
std::function<void(char *, uint8_t *, unsigned int)> __callback;

void connect_mqtt_server()
{
    mqtt_client.setServer(mqtt_broker.c_str(), mqtt_port);
    mqtt_client.setCallback(__callback);

    int tried_count = 0;
    while (!mqtt_client.connected() && tried_count <= 3)
    {
        std::string client_id = "esp32-client-";
        client_id += WiFi.macAddress().c_str();

        Serial.printf("Client %s is connecting to mqtt broker %s ...\r\n", client_id.c_str(), mqtt_broker.c_str());

        if (mqtt_client.connect(client_id.c_str(), mqtt_username.c_str(), mqtt_password.c_str()))
        {
            Serial.println("Mqtt broker connected!");
        }
        else
        {
            Serial.print("Mqtt connection failed with state: ");
            Serial.println(mqtt_client.state());

            tried_count++;
        }
    }
}

void mqtt_publish(std::string topic, std::string message)
{
    if (!mqtt_client.connected())
    {
        connect_mqtt_server();
    }

    bool res = mqtt_client.publish(topic.c_str(), message.c_str());

    if (res)
    {
        Serial.printf("Mqtt published one message: %s\r\n", message.c_str());
    } else {
        Serial.println("Mqtt publish failed");
    }
    
}

void mqtt_subscribe(std::string topic)
{
    if (!mqtt_client.connected())
    {
        connect_mqtt_server();
    }

    mqtt_client.subscribe(topic.c_str());
}

void mqtt_init(std::string broker, int port, std::string username, std::string password, MQTT_CALLBACK_SIGNATURE)
{
    mqtt_broker = broker;
    mqtt_port = port;
    mqtt_username = username;
    mqtt_password = password;

    if (callback != NULL)
    {
        Serial.println("Mqtt use customized message callback...");
        __callback = callback;
    } else {
        Serial.println("Mqtt does not set callback...");
    }

    connect_mqtt_server();
}

bool isMqttConnected()
{
    return mqtt_client.connected();
}

#endif
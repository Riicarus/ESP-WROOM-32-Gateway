#ifndef BLE2WIFI_H
#define BLE3WIFI_H

#include <Arduino.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <WiFi_Tool.h>
#include <string>

#define SERVICE_UUID "9c5d32e2-19c2-4533-873c-0bee10fb49cf"
#define CHARACTERISTIC_UUID_RX "9e3f53ba-9901-4ff6-a2a5-1a136608ac1c"
#define CHARACTERISTIC_UUID_TX "2cb3648f-cbcd-493e-a67b-a4e47c18d2e6"

const std::string BLE2WiFi_Prefix = "*WiFi*:";

uint8_t txValue = 0;
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

void BLE2WiFi(std::string value);

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
    }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string rxValue = pCharacteristic->getValue(); //接收信息

        if (rxValue.length() > 0)
        {
            Serial.print("RX Receive: ");
            for (int i = 0; i < rxValue.length(); i++)
            {
                Serial.print(rxValue[i]);
            }
            Serial.println();

            BLE2WiFi(rxValue);
        }
    }
};

void start_BLE()
{
    BLEDevice::init("ESP-WROOM-32-GATEWAY");

    pServer = BLEDevice::createServer();

    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);

    pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->addDescriptor(new BLE2902());
    BLECharacteristic *pRxCharacteristic =
        pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);

    pRxCharacteristic->setCallbacks(new MyCallbacks());


    pService->start();

    pServer->getAdvertising()->start();
    Serial.println("BLE Waiting For New Connection and Message ... ");
}

void handle_BLE_service()
{
    if (deviceConnected)
    {
        pTxCharacteristic->setValue(&txValue, 1);

        pTxCharacteristic->notify();

        txValue++;

        delay(2000);
    }

    if (!deviceConnected && oldDeviceConnected)
    {

        delay(500);

        pServer->startAdvertising();
        Serial.println("BLE Start Adevertising ...");
        oldDeviceConnected = deviceConnected;
    }


    if (deviceConnected && !oldDeviceConnected)
    {
        oldDeviceConnected = deviceConnected;
    }
}

void BLE2WiFi(std::string value)
{
    if (BLE2WiFi_Prefix.compare(value.substr(0, 7)) == 0)
    {
        std::string WiFi_Config = value.substr(7);

        int index = WiFi_Config.find("|");
        std::string name = WiFi_Config.substr(0, index);
        std::string password = WiFi_Config.substr(index + 1);

        // report to gate way if this device is not gateway
        WiFi_Connect(name, password, true);
    }
}

#endif
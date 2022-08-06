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
            //向串口输出收到的值
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
    // 创建一个 BLE 设备
    BLEDevice::init("ESP-WROOM-32-GATEWAY");

    // 创建一个 BLE 服务
    pServer = BLEDevice::createServer();
    // 设置回调
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // 创建一个 BLE 特征
    pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->addDescriptor(new BLE2902());
    BLECharacteristic *pRxCharacteristic =
        pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
    // 设置回调
    pRxCharacteristic->setCallbacks(new MyCallbacks());

    // 开始服务
    pService->start();
    // 开始广播
    pServer->getAdvertising()->start();
    Serial.println("BLE Waiting For New Connection and Message ... ");
}

void handle_BLE_service()
{
    // deviceConnected 已连接
    if (deviceConnected)
    {
        // 设置要发送的值为1
        pTxCharacteristic->setValue(&txValue, 1);
        // 广播
        pTxCharacteristic->notify();
        // 指针地址自加1
        txValue++;
        // 如果有太多包要发送，蓝牙会堵塞
        delay(2000);
    }

    // disconnecting  断开连接
    if (!deviceConnected && oldDeviceConnected)
    {
        // 留时间给蓝牙缓冲
        delay(500);
        // 重新广播
        pServer->startAdvertising();
        Serial.println("BLE Start Adevertising ...");
        oldDeviceConnected = deviceConnected;
    }

    // connecting  正在连接
    if (deviceConnected && !oldDeviceConnected)
    {
        // do stuff here on connecting
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
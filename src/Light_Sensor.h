#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include <Arduino.h>

#define LIGHT_SENSOR_PIN 32

void light_sensor_start() {
    pinMode(LIGHT_SENSOR_PIN, INPUT);
}

int readLightSensorData() {
    return digitalRead(LIGHT_SENSOR_PIN);
}

#endif
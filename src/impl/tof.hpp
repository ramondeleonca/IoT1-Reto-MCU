#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>

// #define SENSOR_1_ONLY

constexpr uint8_t lox1xshut = 18;
constexpr uint8_t lox1addr = 0x30;
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
VL53L0X_RangingMeasurementData_t lox1measure;

constexpr uint8_t lox2xshut = 19;
constexpr uint8_t lox2addr = 0x31;
#ifndef SENSOR_1_ONLY
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();
VL53L0X_RangingMeasurementData_t lox2measure;
#endif

constexpr float sensorDistanceMM = 1.0 /* inch */ * 2.54 /* to cm */ * 10 /* to mm */;

float getDistanceMM(Adafruit_VL53L0X &lox) {
    VL53L0X_RangingMeasurementData_t measure{};
    if (lox.rangingTest(&measure, false) == VL53L0X_ERROR_NONE &&
        measure.RangeStatus == 0) {
        return measure.RangeMilliMeter;
    }

    return -1; // Indicate an invalid or out-of-range measurement.
}

void setup() {
    Serial.begin(115200);

    pinMode(lox1xshut, OUTPUT);
    digitalWrite(lox1xshut, LOW);
    
    pinMode(lox2xshut, OUTPUT);
    digitalWrite(lox2xshut, LOW);

    Wire.begin();

    digitalWrite(lox1xshut, HIGH);
    delay(10);
    if (!lox1.begin()) Serial.println("Error initializing sensor 1.");
    else if (!lox1.setAddress(lox1addr)) Serial.println("Error setting sensor 1 I2C address.");
    else if (!lox1.configSensor(Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_SPEED)) Serial.println("Error configuring sensor 1.");

    #ifndef SENSOR_1_ONLY
    digitalWrite(lox2xshut, HIGH);
    delay(10);
    if (!lox2.begin()) Serial.println("Error initializing sensor 2.");
    else if (!lox2.setAddress(lox1addr)) Serial.println("Error setting sensor 2 I2C address.");
    else if (!lox2.configSensor(Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_SPEED)) Serial.println("Error configuring sensor 2.");
    #endif
}

void loop() {
    float lox1Dist = getDistanceMM(lox1);
    #ifndef SENSOR_1_ONLY
    float lox2Dist = getDistanceMM(lox2);
    #endif

    Serial.print(lox1Dist);
    Serial.print("mm");

    Serial.print(", ");

    #ifndef SENSOR_1_ONLY
    Serial.print(lox2Dist);
    Serial.print("mm");
    #endif
    
    Serial.println();

    delay(10);
}
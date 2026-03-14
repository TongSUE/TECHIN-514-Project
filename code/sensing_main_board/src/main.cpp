#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include "Adafruit_TCS34725.h"

#define PIN_BUTTON D0
#define PIN_ILLUM_LED D2

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

bool sensorOnline = false;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
typedef struct struct_message { uint16_t r, g, b, c; } struct_message;
struct_message colorData;
esp_now_peer_info_t peerInfo;

bool checkI2CConnection() {
    Wire.beginTransmission(0x29);
    return (Wire.endTransmission() == 0);
}

void tryInitSensor() {
    Serial.println("Attempting to reconnect to TCS34725...");
    if (tcs.begin()) {
        Serial.println(">> SUCCESS: Sensor Connected!");
        sensorOnline = true;
    } else {
        Serial.println(">> FAILED: Sensor not found. Please check wiring (SDA/SCL).");
        sensorOnline = false;
    }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "ESP-NOW: TX OK" : "ESP-NOW: TX FAIL");
}

void setup() {
    Serial.begin(115200);
    delay(3000); 
    
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_ILLUM_LED, OUTPUT);
    digitalWrite(PIN_ILLUM_LED, LOW);

    Wire.begin();
    Wire.setClock(100000); 

    tryInitSensor();

    WiFi.mode(WIFI_STA);
    if (esp_now_init() == ESP_OK) {
        esp_now_register_send_cb(OnDataSent);
        memcpy(peerInfo.peer_addr, broadcastAddress, 6);
        peerInfo.channel = 0;  
        peerInfo.encrypt = false;
        esp_now_add_peer(&peerInfo);
    }
}

unsigned long lastCheckTime = 0;

void loop() {
    unsigned long now = millis();

    if (now - lastCheckTime > 2000) {
        lastCheckTime = now;
        if (!checkI2CConnection()) {
            if (sensorOnline) {
                Serial.println("\n[!] ALERT: Sensor Disconnected!");
                sensorOnline = false;
            }
            tryInitSensor();
        }
    }

    if (sensorOnline && digitalRead(PIN_BUTTON) == LOW) {
        delay(50); 
        if (digitalRead(PIN_BUTTON) == LOW) {
            Serial.println("\n--- Triggered Scan ---");
            
            digitalWrite(PIN_ILLUM_LED, HIGH);
            delay(100); 

            if (checkI2CConnection()) {
                tcs.getRawData(&colorData.r, &colorData.g, &colorData.b, &colorData.c);
                digitalWrite(PIN_ILLUM_LED, LOW);

                Serial.printf("RAW -> R:%d G:%d B:%d C:%d\n", colorData.r, colorData.g, colorData.b, colorData.c);

                if (colorData.c > 0) {
                    esp_now_send(broadcastAddress, (uint8_t *) &colorData, sizeof(colorData));
                }
            } else {
                Serial.println("[ERROR] Bus failed during scan!");
                digitalWrite(PIN_ILLUM_LED, LOW);
                sensorOnline = false;
            }

            while (digitalRead(PIN_BUTTON) == LOW) delay(10);
            Serial.println("Ready.\n");
        }
    }
}
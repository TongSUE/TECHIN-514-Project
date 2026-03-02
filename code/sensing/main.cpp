#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_AS7341.h>

// Pin definitions (Update according to custom PCB schematic)
#define PIN_BUTTON D1
#define PIN_ILLUM_LED D2

Adafruit_AS7341 as7341;

// Base station MAC address (Replace with actual MAC)
uint8_t baseMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Data packet structure matching AS7341 spectral channels
typedef struct struct_message {
    uint16_t f1, f2, f3, f4, f5, f6, f7, f8;
    uint16_t clear, nir;
} struct_message;

struct_message sensorData;
esp_now_peer_info_t peerInfo;

// ESP-NOW callback
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "TX Success" : "TX Fail");
}

void setup() {
    Serial.begin(115200);
    
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_ILLUM_LED, OUTPUT);
    digitalWrite(PIN_ILLUM_LED, LOW);

    // Init I2C and AS7341
    if (!as7341.begin()) {
        Serial.println("AS7341 init failed");
        while (1); 
    }
    
    // Configure sensor settings
    as7341.setATIME(100);
    as7341.setASTEP(999);
    as7341.setGain(AS7341_GAIN_256X);

    // Init ESP-NOW
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return;
    }
    esp_now_register_send_cb(onDataSent);

    // Register Base peer
    memcpy(peerInfo.peer_addr, baseMacAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
}

void loop() {
    // Read button (active low)
    if (digitalRead(PIN_BUTTON) == LOW) {
        delay(50); // Debounce
        if (digitalRead(PIN_BUTTON) == LOW) {
            
            // 1. Turn on illumination
            digitalWrite(PIN_ILLUM_LED, HIGH);
            delay(100); // Allow light and sensor to stabilize

            // 2. Read spectral data
            if (as7341.readAllChannels()) {
                sensorData.f1 = as7341.getChannel(AS7341_CHANNEL_415nm_F1);
                sensorData.f2 = as7341.getChannel(AS7341_CHANNEL_445nm_F2);
                sensorData.f3 = as7341.getChannel(AS7341_CHANNEL_480nm_F3); // Blue
                sensorData.f4 = as7341.getChannel(AS7341_CHANNEL_515nm_F4); // Green
                sensorData.f5 = as7341.getChannel(AS7341_CHANNEL_555nm_F5);
                sensorData.f6 = as7341.getChannel(AS7341_CHANNEL_590nm_F6);
                sensorData.f7 = as7341.getChannel(AS7341_CHANNEL_630nm_F7); // Red
                sensorData.f8 = as7341.getChannel(AS7341_CHANNEL_680nm_F8); // Red
                sensorData.clear = as7341.getChannel(AS7341_CHANNEL_CLEAR);
                sensorData.nir = as7341.getChannel(AS7341_CHANNEL_NIR);

                // 3. Send via ESP-NOW
                esp_now_send(baseMacAddress, (uint8_t *)&sensorData, sizeof(sensorData));
            }
            
            // 4. Turn off illumination
            digitalWrite(PIN_ILLUM_LED, LOW);

            // Wait for button release
            while(digitalRead(PIN_BUTTON) == LOW); 
        }
    }
}
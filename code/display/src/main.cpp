#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include "pantone_colors.h"

#define PIN_NEO D9

const int M0 = 2;  
const int M1 = 4;  
const int M2 = 1;  
const int M3 = 3;  

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
Adafruit_NeoPixel pixels(2, PIN_NEO, NEO_GRB + NEO_KHZ800);

const int stepSeq[4][4] = {
  {1, 0, 1, 0},
  {0, 1, 1, 0},
  {0, 1, 0, 1},
  {1, 0, 0, 1}
};

int currentStep = 0;
int targetStep = 0;
unsigned long lastMotorTick = 0;
const int motorSpeedDelay = 3; 

const int POS_LOW  = 167; 
const int POS_HIGH = 433; 

// --- ESP-NOW Data Structure ---
typedef struct struct_message {
    uint16_t r;
    uint16_t g;
    uint16_t b;
    uint16_t c;
} struct_message;

struct_message incomingData;
volatile bool newDataReady = false;

// ESP-NOW Receive Callback
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingDataPtr, int len) {
    memcpy(&incomingData, incomingDataPtr, sizeof(incomingData));
    newDataReady = true; 
}

void updateOLED(uint8_t scannedR, uint8_t scannedG, uint8_t scannedB, float conf, String pantoneName) {
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.setCursor(0, 0);
    
    oled.print("Scanned RGB: ");
    oled.print(scannedR); oled.print(",");
    oled.print(scannedG); oled.print(",");
    oled.println(scannedB);
    oled.println("");

    oled.print("Confidence:  ");
    oled.print(conf, 1); 
    oled.println("%");
    oled.println(""); 

    oled.print("Match: ");
    oled.println(pantoneName);
    oled.println(""); 

    oled.println("[Ready]");
    
    oled.display();
}

void updateNeoPixels(int statusIndicator, uint8_t resultR, uint8_t resultG, uint8_t resultB) {
    switch(statusIndicator) {
        case 0: pixels.setPixelColor(0, pixels.Color(255, 255, 255)); break; 
        case 1: pixels.setPixelColor(0, pixels.Color(0, 0, 255)); break;     
        case 2: pixels.setPixelColor(0, pixels.Color(255, 0, 0)); break;     
        case 3: pixels.setPixelColor(0, pixels.Color(255, 255, 0)); break;   
        case 4: pixels.setPixelColor(0, pixels.Color(0, 255, 0)); break;     
    }

    pixels.setPixelColor(1, pixels.Color(resultR, resultG, resultB));
    pixels.show();
}

void zeroMotor() {
    Serial.println("Calibrating motor to 0...");
    for (int i = 0; i < 600; i++) {
        currentStep--;
        int stepIndex = (currentStep % 4 + 4) % 4; 
        digitalWrite(M0, stepSeq[stepIndex][0]);
        digitalWrite(M1, stepSeq[stepIndex][1]);
        digitalWrite(M2, stepSeq[stepIndex][2]);
        digitalWrite(M3, stepSeq[stepIndex][3]);
        delay(motorSpeedDelay); 
    }
    digitalWrite(M0, LOW); digitalWrite(M1, LOW); 
    digitalWrite(M2, LOW); digitalWrite(M3, LOW);
    currentStep = 0; 
}

void setMotorConfidence(float confidencePercent) {
    confidencePercent = constrain(confidencePercent, 0.0, 100.0);
    zeroMotor(); 
    targetStep = map(confidencePercent, 0, 100, POS_LOW, POS_HIGH); 
}

void runMotorTick() {
    if (millis() - lastMotorTick >= motorSpeedDelay) { 
        if (currentStep != targetStep) {
            if (currentStep < targetStep) currentStep++;
            else currentStep--;
            int stepIndex = (currentStep % 4 + 4) % 4; 
            digitalWrite(M0, stepSeq[stepIndex][0]);
            digitalWrite(M1, stepSeq[stepIndex][1]);
            digitalWrite(M2, stepSeq[stepIndex][2]);
            digitalWrite(M3, stepSeq[stepIndex][3]);
            lastMotorTick = millis();
        } else {
            digitalWrite(M0, LOW); digitalWrite(M1, LOW); 
            digitalWrite(M2, LOW); digitalWrite(M3, LOW);
        }
    }
}

void performKNNAndOutput(uint8_t scannedR, uint8_t scannedG, uint8_t scannedB) {
    int bestIndex = 0;
    float minDistance = 1000.0; 

    // KNN: compute distance to each Pantone color and find the closest match
    for (int i = 0; i < dbSize; i++) {
        float distance = sqrt(pow(scannedR - pantoneDB[i].r, 2) + 
                              pow(scannedG - pantoneDB[i].g, 2) + 
                              pow(scannedB - pantoneDB[i].b, 2));
        if (distance < minDistance) {
            minDistance = distance;
            bestIndex = i;
        }
    }

    String matchedName = pantoneDB[bestIndex].name;
    float confidence = (1.0 - (minDistance / 441.6)) * 100.0;
    
    int statusLevel;
    if (confidence < 40) statusLevel = 2;      
    else if (confidence < 80) statusLevel = 3; 
    else statusLevel = 4;                      

    Serial.printf("KNN Matched: %s | Conf: %.1f%%\n", matchedName.c_str(), confidence);

    // update outputs
    setMotorConfidence(confidence);
    updateOLED(scannedR, scannedG, scannedB, confidence, matchedName);
    updateNeoPixels(statusLevel, scannedR, scannedG, scannedB);
}

void boostSaturation(uint8_t &r, uint8_t &g, uint8_t &b, float factor) {
    // compute luminance and boost saturation by pushing RGB away from the luminance center
    float luminance = (r + g + b) / 3.0;
    
    r = constrain(luminance + (r - luminance) * factor, 0, 200);
    g = constrain(luminance + (g - luminance) * factor, 0, 200);
    b = constrain(luminance + (b - luminance) * factor, 0, 200);
}

void handleInputTuple(uint16_t rawR, uint16_t rawG, uint16_t rawB, uint16_t rawC) {
    if (rawC == 0) return; 
    // Normalize RGB values to 0-255 range based on rawC, then boost saturation for better KNN performance
    uint8_t normR = constrain((rawR * 255) / rawC, 0, 255);
    uint8_t normG = constrain((rawG * 255) / rawC, 0, 255);
    uint8_t normB = constrain((rawB * 255) / rawC, 0, 255);
    
    boostSaturation(normR, normG, normB, 3.0); 
    
    performKNNAndOutput(normR, normG, normB);
}

void setup() {
    Serial.begin(115200);
    
    pinMode(M0, OUTPUT); pinMode(M1, OUTPUT); 
    pinMode(M2, OUTPUT); pinMode(M3, OUTPUT);

    pixels.begin();
    updateNeoPixels(0, 0, 0, 0); 

    Wire.begin(); 
    if(!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed."));
    } else {
        oled.clearDisplay();
        
    
        oled.setTextSize(1);
        oled.setTextColor(WHITE);
        oled.setCursor(0, 0);
        oled.println("System Booting...");
        oled.println("");
        oled.println("");
        oled.println("");
        oled.println("[Ready]");
        oled.display();
    }
    

    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(OnDataRecv);

    delay(1000); 
    updateNeoPixels(1, 0, 0, 0);
    Serial.println("Display System Ready. Listening for ESP-NOW...");
}

void loop() {
    runMotorTick(); 

    if (newDataReady) {
        newDataReady = false;
        Serial.printf("\nReceived RX Packet -> R:%d G:%d B:%d C:%d\n", 
                      incomingData.r, incomingData.g, incomingData.b, incomingData.c);
        
        handleInputTuple(incomingData.r, incomingData.g, incomingData.b, incomingData.c);
    }
}
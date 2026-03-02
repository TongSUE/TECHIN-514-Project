#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <Stepper.h>
#include <math.h>

// Pin definitions (Update according to custom PCB schematic)
#define PIN_WS2812B D3
#define NUM_PIXELS  1
#define PIN_MOTOR_IN1 D7
#define PIN_MOTOR_IN2 D8
#define PIN_MOTOR_IN3 D9
#define PIN_MOTOR_IN4 D10

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Hardware objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_NeoPixel strip(NUM_PIXELS, PIN_WS2812B, NEO_GRB + NEO_KHZ800);
Stepper stepper(2048, PIN_MOTOR_IN1, PIN_MOTOR_IN3, PIN_MOTOR_IN2, PIN_MOTOR_IN4);

// Incoming data structure (Must match Pen)
typedef struct struct_message {
    uint16_t f1, f2, f3, f4, f5, f6, f7, f8;
    uint16_t clear, nir;
} struct_message;

struct_message incomingData;
bool newData = false;

// --- Pantone Library for KNN ---
struct PantoneColor {
    const char* name;
    uint8_t r, g, b;
};

// Mini Pantone database (Add more colors to improve accuracy)
PantoneColor pantoneDB[] = {
    {"Pantone 5493 C", 137, 172, 172},
    {"Warm Red C", 249, 66, 58},
    {"Reflex Blue C", 0, 20, 137},
    {"Yellow C", 254, 223, 0},
    {"Black 6 C", 16, 24, 32},
    {"White", 255, 255, 255}
};
const int dbSize = sizeof(pantoneDB) / sizeof(pantoneDB[0]);

// ESP-NOW callback
void onDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
    memcpy(&incomingData, data, sizeof(incomingData));
    newData = true;
}

void setup() {
    Serial.begin(115200);

    // Init OLED
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED init failed");
        for(;;);
    }
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("Waiting...");
    display.display();

    // Init LED
    strip.begin();
    strip.clear();
    strip.show();

    // Init Stepper
    stepper.setSpeed(10); // 10 RPM

    // Init ESP-NOW
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) return;
    esp_now_register_recv_cb(onDataRecv);
}

// Convert RGB to CMYK
void calculateCMYK(uint8_t r, uint8_t g, uint8_t b, float &c, float &m, float &y, float &k) {
    if (r == 0 && g == 0 && b == 0) {
        c = m = y = 0; k = 1; return;
    }
    float rP = r / 255.0; float gP = g / 255.0; float bP = b / 255.0;
    k = 1.0 - max(max(rP, gP), bP);
    c = (1.0 - rP - k) / (1.0 - k);
    m = (1.0 - gP - k) / (1.0 - k);
    y = (1.0 - bP - k) / (1.0 - k);
}

void loop() {
    if (newData) {
        newData = false;

        // 1. Convert Spectral Data to RGB (Simplified approximation)
        // Adjust weightings based on actual testing/calibration
        float redCalc = incomingData.f7 + incomingData.f8; 
        float greenCalc = incomingData.f4 + incomingData.f5;
        float blueCalc = incomingData.f2 + incomingData.f3;
        
        // Normalize using Clear channel (Example mapping, needs tuning)
        uint8_t r = constrain(map(redCalc, 0, incomingData.clear, 0, 255), 0, 255);
        uint8_t g = constrain(map(greenCalc, 0, incomingData.clear, 0, 255), 0, 255);
        uint8_t b = constrain(map(blueCalc, 0, incomingData.clear, 0, 255), 0, 255);

        // 2. Format HEX
        char hexCode[8];
        sprintf(hexCode, "#%02X%02X%02X", r, g, b);

        // 3. Convert to CMYK
        float cy, mg, yl, bk;
        calculateCMYK(r, g, b, cy, mg, yl, bk);

        // 4. KNN for Nearest Pantone Color
        int bestMatchIndex = 0;
        float minDistance = 99999.0;

        for (int i = 0; i < dbSize; i++) {
            // Euclidean distance in RGB space
            float dist = sqrt(pow(r - pantoneDB[i].r, 2) + 
                              pow(g - pantoneDB[i].g, 2) + 
                              pow(b - pantoneDB[i].b, 2));
            if (dist < minDistance) {
                minDistance = dist;
                bestMatchIndex = i;
            }
        }

        // 5. Update OLED
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println(pantoneDB[bestMatchIndex].name);
        
        display.print("HEX: "); display.println(hexCode);
        
        display.print("RGB: "); 
        display.print(r); display.print(","); display.print(g); display.print(","); display.println(b);
        
        display.print("CMYK: ");
        display.print((int)(cy*100)); display.print(",");
        display.print((int)(mg*100)); display.print(",");
        display.print((int)(yl*100)); display.print(",");
        display.println((int)(bk*100));
        
        display.display();

        // 6. Update WS2812B NeoPixel
        strip.setPixelColor(0, strip.Color(r, g, b));
        strip.show();

        // 7. Update Stepper (Map confidence/distance to gauge)
        // If distance is 0, it's a 100% match. 
        int stepsToMove = map(minDistance, 0, 255, 512, 0); // 512 is 90 degrees
        stepsToMove = constrain(stepsToMove, 0, 512);
        
        stepper.step(stepsToMove);
        delay(2000);
        stepper.step(-stepsToMove); // Return to zero
    }
}
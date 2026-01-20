# TECHIN-514-Project : The Reality Color Dropper

## 1. Project Overview

**What it does:**
This project brings the digital "eyedropper" tool into real life. It consists of a pen-shaped device that scans the color of physical objects and a desktop station that displays the color data and how accurate the match is. It helps designers capture colors from the real world instantly.

<img src="./images/overview.png" width="400" alt="Sensor device">

---

## 2. The Sensor Device

**Description:**
This is a handheld tool shaped like a marker pen. When I find a color I like, I press the button (like a camera shutter). The device reads the RGB value, blocks outside light for accuracy, and sends the data wirelessly to the base station.

<img src="./images/sensing.png" width="300" alt="Sensor device">

**Key Components & Part Numbers:**

* **Controller: Seeed Studio XIAO ESP32C3**
It is extremely small (thumb-sized), fitting perfectly inside a pen. It also handles the wireless communication.


* **Sensor: Custom PCB with TCS34725**
The TCS34725 is a good RGB sensor. I will design a slim PCB to fit it into the pen tip along with an LED for consistent lighting.


* **Input: Tactile Button**
Acts as a "shutter" trigger. Press to scan.


* **Power: Li-Po Battery (402030 size)**
Small form factor to fit the enclosure.


* **Power Switch: Slide Switch**
To cut off power completely when not in use.


* **LED Indicator**
Shows if the device is on and if the data was sent successfully.



---

## 3. The Display Device

**Description:**
This is a desktop base that receives the color data. It has an OLED screen to show the exact color code (HEX/RGB) and a stepper motor gauge that points to the "Confidence Level" (how close the match is). It also has a bright RGB LED to visually recreate the scanned color.

<img src="./images/display.png" width="500" alt="Display device">

**Key Components & Part Numbers:**

* **Controller: ESP32-WROOM-32 DevKit**
Standard board with enough pins to control the motor, screen, and LEDs at the same time.


* **Display: 0.96" OLED Screen (SSD1306 Driver)**
To display text data like Hex codes clearly. Uses I2C interface.


* **Color Output: WS2812B RGB LED**
A programmable LED to visually show the color we just scanned.


* **Motor: 28BYJ-48 Stepper Motor**
A geared motor that can hold the pointer position precisely for the gauge.


* **Motor Driver: ULN2003 Driver Board**
The ESP32 cannot power the motor directly. This driver handles the high current needed for the motor.


* **Mode Button**
To switch display modes (e.g., between RGB and Pantone).


* **Power: 18650 Li-Ion Battery**
High capacity to power the motor and LEDs for a long time.


* **LED Indicator**
Shows power status and connection status.



---

## 4. System Logic & Communication

**Communication Method:**
The devices talk to each other using **ESP-NOW**.

* It is a peer-to-peer protocol (like a walkie-talkie).
* It is much faster than standard Wi-Fi and doesn't need a router or pairing process.

<img src="./images/communication.png" height="200" alt="Display device">

**Workflow:**

1. **Trigger:** User clicks the pen button.
2. **Sensing:** The TCS34725 sensor reads the Raw RGB data.
3. **Sending:** The Pen (ESP32C3) processes the data and sends it via ESP-NOW.
4. **Receiving:** The Base (ESP32-WROOM) receives the packet.
5. **Processing (DSP/ML):** The Base calculates the **Euclidean Distance** between the scanned color and a stored color library (KNN algorithm) to find the closest match.
6. **Output:**
* The **OLED** shows the color code.
* The **WS2812B LED** lights up in that color.
* The **Stepper Motor** moves the needle to show how confident the match is (High vs. Low accuracy).

<img src="./images/workflow.png" height="200" alt="Display device">
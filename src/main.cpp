#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <SPI.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>
#include "secrets.h"

const int LED_PIN = 2; 
#define OLED_DC     26   
#define OLED_CS     14   
#define OLED_RESET  25   

// Joystick pins
#define JOYSTICK_SW  16 // Button
#define JOYSTICK_VRY 35  // Y axis (vertical)
#define JOYSTICK_VRX 34  // X axis (horizontal)

Adafruit_SSD1306 display(128, 64, &SPI, OLED_DC, OLED_RESET, OLED_CS);

unsigned long previousMillis = 0;
int blinkInterval = 1000;
bool isConnected = false;
bool timeConfigured = false;

// Menu and joystick state
int selectedMenuItem = 0;
unsigned long lastJoystickRead = 0;
const int JOYSTICK_DEBOUNCE_MS = 50;
const int JOYSTICK_THRESHOLD = 1500;
bool lastButtonState = true;

void handleJoystickInput() {
    unsigned long currentTime = millis();

    int vrx = analogRead(JOYSTICK_VRX);
    int vry = analogRead(JOYSTICK_VRY);
    bool buttonPressed = digitalRead(JOYSTICK_SW) == LOW;

    // Y-axis navigation
    if (vry < JOYSTICK_THRESHOLD) {
        if (currentTime - lastJoystickRead >= JOYSTICK_DEBOUNCE_MS) {
            selectedMenuItem = (selectedMenuItem + 1) % 3;
            lastJoystickRead = currentTime;
            Serial.println("Joystick Down - Selected: " + String(selectedMenuItem));
        }
    }
    else if (vry > 4095 - JOYSTICK_THRESHOLD) {
        if (currentTime - lastJoystickRead >= JOYSTICK_DEBOUNCE_MS) {
            selectedMenuItem = (selectedMenuItem - 1 + 3) % 3;
            lastJoystickRead = currentTime;
            Serial.println("Joystick Up - Selected: " + String(selectedMenuItem));
        }
    }

    // Button press
    if (buttonPressed && lastButtonState) {
        lastButtonState = false;
        Serial.println("Button pressed on menu item: " + String(selectedMenuItem));
        switch (selectedMenuItem) {
            case 0: Serial.println("Selected: WiFi"); break;
            case 1: Serial.println("Selected: BT Scan"); break;
            case 2: Serial.println("Selected: Settings"); break;
        }
    }
    else if (!buttonPressed && !lastButtonState) {
        lastButtonState = true;
    }
}

void drawWiFiIcon(int x, int y, int strength) {
    display.fillCircle(x, y, 1, SSD1306_WHITE);
    
    if (strength >= 1) {
        display.drawPixel(x-1, y-1, SSD1306_WHITE);
        display.drawPixel(x+1, y-1, SSD1306_WHITE);
        display.drawPixel(x-2, y-2, SSD1306_WHITE);
        display.drawPixel(x+2, y-2, SSD1306_WHITE);
    }
    
    if (strength >= 2) {
        display.drawPixel(x-2, y-3, SSD1306_WHITE);
        display.drawPixel(x+2, y-3, SSD1306_WHITE);
        display.drawPixel(x-3, y-4, SSD1306_WHITE);
        display.drawPixel(x+3, y-4, SSD1306_WHITE);
        display.drawPixel(x-4, y-5, SSD1306_WHITE);
        display.drawPixel(x+4, y-5, SSD1306_WHITE);
    }
    
    if (strength >= 3) {
        display.drawPixel(x-4, y-6, SSD1306_WHITE);
        display.drawPixel(x+4, y-6, SSD1306_WHITE);
        display.drawPixel(x-5, y-7, SSD1306_WHITE);
        display.drawPixel(x+5, y-7, SSD1306_WHITE);
        display.drawPixel(x-6, y-8, SSD1306_WHITE);
        display.drawPixel(x+6, y-8, SSD1306_WHITE);
    }
    
    if (strength >= 4) {
        display.drawPixel(x-6, y-9, SSD1306_WHITE);
        display.drawPixel(x+6, y-9, SSD1306_WHITE);
        display.drawPixel(x-7, y-10, SSD1306_WHITE);
        display.drawPixel(x+7, y-10, SSD1306_WHITE);
        display.drawPixel(x-8, y-11, SSD1306_WHITE);
        display.drawPixel(x+8, y-11, SSD1306_WHITE);
    }
}

void drawHomeScreen() {
    display.clearDisplay();
    
    display.drawFastHLine(0, 12, 128, SSD1306_WHITE);
    
    if (WiFi.status() == WL_CONNECTED) {
        int rssi = WiFi.RSSI();
        int strength;
        
        if (rssi >= -50) strength = 4;
        else if (rssi >= -60) strength = 3;
        else if (rssi >= -70) strength = 2;
        else strength = 1;
        
        drawWiFiIcon(10, 10, strength);
    } else {
        display.setTextSize(1);
        display.setCursor(6, 2);
        display.print("X");
    }
    
    display.setTextSize(1);
    display.setCursor(50, 2);
    time_t now = time(nullptr);
    if (now > 1577836800) {
        struct tm* timeinfo = localtime(&now);
        char time_str[6];
        strftime(time_str, sizeof(time_str), "%H:%M", timeinfo);
        display.print(time_str);
    } else {
        display.print("--:--");
    }
    
    display.setCursor(95, 2);
    display.print("100%");

    
    // Menu item 0: WiFi
    display.setCursor(5, 35);
    if (selectedMenuItem == 0) {
        display.fillRect(1, 33, 50, 8, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
    }
    display.print("WiFi");
    display.setTextColor(SSD1306_WHITE);
    
    // Menu item 1: BT Scan
    display.setCursor(5, 45);
    if (selectedMenuItem == 1) {
        display.fillRect(1, 43, 50, 8, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
    }
    display.print("BT Scan");
    display.setTextColor(SSD1306_WHITE);
    
    // Menu item 2: Settings
    display.setCursor(5, 55);
    if (selectedMenuItem == 2) {
        display.fillRect(1, 53, 50, 8, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
    }
    display.print("Settings");
    display.setTextColor(SSD1306_WHITE);
    
    display.display();
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    
    pinMode(JOYSTICK_SW, INPUT_PULLUP);
    analogSetPinAttenuation(JOYSTICK_VRX, ADC_11db);
    analogSetPinAttenuation(JOYSTICK_VRY, ADC_11db);

    SPI.begin(18, 19, 23, OLED_CS); 

    if(!display.begin(SSD1306_SWITCHCAPVCC)) {
        Serial.println("SSD1306 failed");
        for(;;);
    }
    
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    
    display.drawFastHLine(0, 12, 128, SSD1306_WHITE);
    
    display.setCursor(6, 2);
    display.print("Offline");
    
    display.setCursor(50, 2);
    display.print("--:--");
    
    display.setCursor(95, 2);
    display.print("100%");
    
    display.setCursor(5, 35);
    display.print("WiFi");
    display.setCursor(5, 45);
    display.print("BT Scan");
    display.setCursor(5, 55);
    display.print("Settings");
    
    display.display();
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    Serial.println("Setup complete - WiFi connecting in background");
}

void loop() {
    static unsigned long last_screen_update = 0;
    unsigned long currentMillis = millis();
    wl_status_t status = WiFi.status();
    
    handleJoystickInput();

    if (status == WL_CONNECTED) {
        digitalWrite(LED_PIN, HIGH);
        
        if (!isConnected) {
            ArduinoOTA.begin();
            isConnected = true;
            Serial.println("WiFi Connected!");
            Serial.print("RSSI: ");
            Serial.println(WiFi.RSSI());
            drawHomeScreen();
        }
        
        if (!timeConfigured) {
            Serial.println("Configuring time...");
            configTime(2 * 3600, 3600, "pool.ntp.org", "time.nist.gov");
            timeConfigured = true;
            Serial.println("Waiting for NTP sync (background)");
        }
        
        ArduinoOTA.handle();
        
        if (currentMillis - last_screen_update >= 1000) {
            last_screen_update = currentMillis;
            drawHomeScreen();
        }

    } else {
        if (isConnected) {
            isConnected = false;
            timeConfigured = false;
            Serial.println("WiFi disconnected");
            drawHomeScreen();
        }

        blinkInterval = (status == WL_DISCONNECTED || status == WL_IDLE_STATUS) ? 500 : 1500;

        if (currentMillis - previousMillis >= blinkInterval) {
            previousMillis = currentMillis;
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        }
        
        if (currentMillis - last_screen_update >= 2000) {
            last_screen_update = currentMillis;
            drawHomeScreen();
        }
    }
}
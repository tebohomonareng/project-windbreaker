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

Adafruit_SSD1306 display(128, 64, &SPI, OLED_DC, OLED_RESET, OLED_CS);

unsigned long previousMillis = 0;
int blinkInterval = 1000;
bool isConnected = false;
bool timeConfigured = false;

// Draw WiFi icon with curved arcs
void drawWiFiIcon(int x, int y, int strength) {
    // Draw dot at bottom (always on if connected)
    display.fillCircle(x, y, 1, SSD1306_WHITE);
    
    // Draw arcs based on signal strength (1-4)
    if (strength >= 1) {
        // First arc (smallest/closest)
        display.drawPixel(x-1, y-1, SSD1306_WHITE);
        display.drawPixel(x+1, y-1, SSD1306_WHITE);
        display.drawPixel(x-2, y-2, SSD1306_WHITE);
        display.drawPixel(x+2, y-2, SSD1306_WHITE);
    }
    
    if (strength >= 2) {
        // Second arc
        display.drawPixel(x-2, y-3, SSD1306_WHITE);
        display.drawPixel(x+2, y-3, SSD1306_WHITE);
        display.drawPixel(x-3, y-4, SSD1306_WHITE);
        display.drawPixel(x+3, y-4, SSD1306_WHITE);
        display.drawPixel(x-4, y-5, SSD1306_WHITE);
        display.drawPixel(x+4, y-5, SSD1306_WHITE);
    }
    
    if (strength >= 3) {
        // Third arc
        display.drawPixel(x-4, y-6, SSD1306_WHITE);
        display.drawPixel(x+4, y-6, SSD1306_WHITE);
        display.drawPixel(x-5, y-7, SSD1306_WHITE);
        display.drawPixel(x+5, y-7, SSD1306_WHITE);
        display.drawPixel(x-6, y-8, SSD1306_WHITE);
        display.drawPixel(x+6, y-8, SSD1306_WHITE);
    }
    
    if (strength >= 4) {
        // Fourth arc (largest/furthest)
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
    
    // Header
    display.drawFastHLine(0, 12, 128, SSD1306_WHITE);
    
    // WiFi strength indicator
    if (WiFi.status() == WL_CONNECTED) {
        int rssi = WiFi.RSSI();
        int strength;
        
        // Map RSSI to strength (1-4 bars)
        if (rssi >= -50) strength = 4;      // Excellent
        else if (rssi >= -60) strength = 3; // Good
        else if (rssi >= -70) strength = 2; // Fair
        else if (rssi >= -80) strength = 1; // Weak
        else strength = 1;                   // Very weak
        
        drawWiFiIcon(10, 10, strength);
        
    } else {
        // Not connected - show X
        display.setTextSize(1);
        display.setCursor(6, 2);
        display.print("X");
    }
    
    // Time (center)
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
    
    // Battery (right)
    display.setCursor(95, 2);
    display.print("100%");
    
    // Content area
    display.setCursor(30, 20);
    display.print("ESP32 Ready");
    
    display.setCursor(5, 35);
    display.print("WiFi");
    display.setCursor(5, 45);
    display.print("BT Scan");
    display.setCursor(5, 55);
    display.print("Settings");
    
    display.display();
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    SPI.begin(18, 19, 23, OLED_CS); 

    if(!display.begin(SSD1306_SWITCHCAPVCC)) {
        Serial.println("SSD1306 failed");
        for(;;);
    }
    
    // Show initial screen immediately
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    
    // Draw header
    display.drawFastHLine(0, 12, 128, SSD1306_WHITE);
    
    // Show X for WiFi (not connected yet)
    display.setCursor(6, 2);
    display.print("Offline");
    
    // Show time placeholder
    display.setCursor(50, 2);
    display.print("--:--");
    
    // Show battery
    display.setCursor(95, 2);
    display.print("100%");
    
    // Content
    display.setCursor(30, 20);
    display.print("ESP32 Ready");
    display.setCursor(5, 35);
    display.print("WiFi");
    display.setCursor(5, 45);
    display.print("BT Scan");
    display.setCursor(5, 55);
    display.print("Settings");
    
    display.display();
    
    // Start WiFi connection in background (non-blocking)
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    Serial.println("Setup complete - WiFi connecting in background");
}

void loop() {
    static unsigned long last_screen_update = 0;
    unsigned long currentMillis = millis();
    wl_status_t status = WiFi.status();

    if (status == WL_CONNECTED) {
        digitalWrite(LED_PIN, HIGH);
        
        if (!isConnected) {
            ArduinoOTA.begin();
            isConnected = true;
            Serial.println("WiFi Connected!");
            Serial.print("RSSI: ");
            Serial.println(WiFi.RSSI());
            
            // Immediately update screen to show WiFi connected
            drawHomeScreen();
        }
        
        if (!timeConfigured) {
            Serial.println("Configuring time...");
            configTime(2 * 3600, 3600, "pool.ntp.org", "time.nist.gov");
            timeConfigured = true;
            
            // Non-blocking time sync check
            Serial.println("Waiting for NTP sync (background)");
        }
        
        ArduinoOTA.handle();
        
        // Update screen every second when connected
        if (currentMillis - last_screen_update >= 1000) {
            last_screen_update = currentMillis;
            drawHomeScreen();
        }

    } else {
        // Not connected
        if (isConnected) {
            isConnected = false;
            timeConfigured = false;
            Serial.println("WiFi disconnected");
            
            // Immediately update screen to show disconnected
            drawHomeScreen();
        }

        // Blink LED based on connection status
        blinkInterval = (status == WL_DISCONNECTED || status == WL_IDLE_STATUS) ? 500 : 1500;

        if (currentMillis - previousMillis >= blinkInterval) {
            previousMillis = currentMillis;
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        }
        
        // Update screen occasionally even when disconnected (every 2 seconds)
        if (currentMillis - last_screen_update >= 2000) {
            last_screen_update = currentMillis;
            drawHomeScreen();
        }
    }
}
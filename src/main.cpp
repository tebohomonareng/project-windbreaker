#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <lvgl.h>
#include <SPI.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "secrets.h"

// --- Pin Definitions ---
const int LED_PIN = 2; 
#define OLED_DC     26   
#define OLED_CS     14   
#define OLED_RESET  25   

Adafruit_SSD1306 display(128, 64, &SPI, OLED_DC, OLED_RESET, OLED_CS);

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[128 * 64];

// UI Elements (global so we can update them)
lv_obj_t * header_bar;
lv_obj_t * wifi_icon;
lv_obj_t * time_label;
lv_obj_t * battery_label;

unsigned long previousMillis = 0;
int blinkInterval = 1000;
bool isConnected = false; 

// Flush function: Maps LVGL pixels to Adafruit pixels
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    for(int16_t y = area->y1; y <= area->y2; y++) {
        for(int16_t x = area->x1; x <= area->x2; x++) {
            display.drawPixel(x, y, color_p->full ? WHITE : BLACK);
            color_p++;
        }
    }
    display.display();
    lv_disp_flush_ready(disp);
}

void create_header_bar() {
    // Create header container
    header_bar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(header_bar, 128, 12);  // Full width, 12px height
    lv_obj_align(header_bar, LV_ALIGN_TOP_MID, 0, 0);
    
    // Style the header
    lv_obj_set_style_radius(header_bar, 0, 0);
    lv_obj_set_style_border_width(header_bar, 0, 0);
    lv_obj_set_style_pad_all(header_bar, 2, 0);
    lv_obj_set_style_bg_color(header_bar, lv_color_black(), 0);
    
    // WiFi icon (left side)
    wifi_icon = lv_label_create(header_bar);
    lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);
    lv_obj_align(wifi_icon, LV_ALIGN_LEFT_MID, 2, 0);
    // lv_obj_set_style_text_font(wifi_icon, &lv_font_montserrat_8, 0);
    
    // Time label (center)
    time_label = lv_label_create(header_bar);
    lv_label_set_text(time_label, "12:34");
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_set_style_text_font(time_label, &lv_font_montserrat_8, 0);
    
    // Battery indicator (right side)
    battery_label = lv_label_create(header_bar);
    lv_label_set_text(battery_label, "100%");
    lv_obj_align(battery_label, LV_ALIGN_RIGHT_MID, -2, 0);
    // lv_obj_set_style_text_font(battery_label, &lv_font_montserrat_8, 0);
}

void create_home_screen() {
    // Create header first
    create_header_bar();
    
    // Main content area (below header)
    lv_obj_t * content = lv_obj_create(lv_scr_act());
    lv_obj_set_size(content, 128, 52);  // 64 - 12 = 52px remaining
    lv_obj_align(content, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_radius(content, 0, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, 4, 0);
    
    // Welcome message or status
    lv_obj_t * welcome_label = lv_label_create(content);
    lv_label_set_text(welcome_label, "ESP32 Ready");
    lv_obj_align(welcome_label, LV_ALIGN_TOP_MID, 0, 5);
    
    // Add some menu options
    lv_obj_t * menu_label = lv_label_create(content);
    lv_label_set_text(menu_label, 
        LV_SYMBOL_WIFI " WiFi\n"
        LV_SYMBOL_BLUETOOTH " BT Scan\n"
        LV_SYMBOL_SETTINGS " Settings");
    lv_obj_align(menu_label, LV_ALIGN_TOP_LEFT, 5, 20);
    // lv_obj_set_style_text_font(menu_label, &lv_font_montserrat_8, 0);
}

// Function to update WiFi icon based on connection status
void update_wifi_status(bool connected) {
    if (wifi_icon != NULL) {
        if (connected) {
            lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);
        } else {
            lv_label_set_text(wifi_icon, "X");
        }
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    SPI.begin(18, 19, 23, OLED_CS); 

    if(!display.begin(SSD1306_SWITCHCAPVCC)) {
        Serial.println("SSD1306 allocation failed");
        for(;;); 
    }
    
    display.clearDisplay();
    display.display();
    
    // LVGL Init
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, 128 * 64);

    // Display Driver Setup
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 128;
    disp_drv.ver_res = 64;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Create the home screen with header
    create_home_screen();
    
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.println("Setup complete");
}

void loop() {
    lv_timer_handler();
    delay(5);
    
    unsigned long currentMillis = millis();
    wl_status_t status = WiFi.status();

    if (status == WL_CONNECTED) {
        digitalWrite(LED_PIN, HIGH); 
        
        if (!isConnected) {
            update_wifi_status(true);
            ArduinoOTA.begin();
            isConnected = true;
            Serial.println("WiFi Connected!");
        }
        ArduinoOTA.handle();

    } else {
        if (isConnected) {
            update_wifi_status(false);
            isConnected = false;
        }

        if (status == WL_DISCONNECTED || status == WL_IDLE_STATUS) {
            blinkInterval = 500; 
        } else {
            blinkInterval = 1500; 
        }

        if (currentMillis - previousMillis >= blinkInterval) {
            previousMillis = currentMillis;
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        }
    }
}
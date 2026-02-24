#include "menu.h"
#include "display/display.h"
#include "attack/attack.h"
#include "attack/sniffer.h"
#include <Arduino.h>

static MenuScreen currentMenu = MENU_HOME;
static HomeMenuItem currentHomeItem = HOME_WIFI;
static WiFiSubmenuItem currentWiFiItem = WIFI_SCAN;
static SettingsSubmenuItem currentSettingsItem = SETTINGS_CHANNEL;
static InfoSubmenuItem currentInfoItem = INFO_DEVICE;
static int selectedItem = 0;

// Scan menu
static NetworkInfo networks[20];
static uint16_t networkCount = 0;
static unsigned long lastScanTime = 0;
static const unsigned long SCAN_INTERVAL = 2000;

// Navigation debouncing
static unsigned long lastBackButtonTime = 0;
static unsigned long lastUpButtonTime = 0;
static unsigned long lastDownButtonTime = 0;
static unsigned long lastLeftButtonTime = 0;
static unsigned long lastRightButtonTime = 0;
static const unsigned long BACK_DEBOUNCE_MS = 300;
static const unsigned long NAV_DEBOUNCE_MS = 200;

void initMenuSystem() {
    Serial.println("[MENU] Menu system initialized");
}

void handleMenuInput(int x, int y, bool button_pressed) {
    unsigned long currentTime = millis();
    
    // Handle button press
    if (button_pressed) {
        switch (currentMenu) {
            case MENU_HOME:
                if (currentHomeItem == HOME_WIFI) {
                    setCurrentMenu(MENU_WIFI_SUBMENU);
                    currentWiFiItem = WIFI_SCAN;
                } else if (currentHomeItem == HOME_SETTINGS) {
                    setCurrentMenu(MENU_SETTINGS_SUBMENU);
                    currentSettingsItem = SETTINGS_CHANNEL;
                } else if (currentHomeItem == HOME_INFO) {
                    setCurrentMenu(MENU_INFO_SUBMENU);
                    currentInfoItem = INFO_DEVICE;
                }
                break;
                
            case MENU_WIFI_SUBMENU:
                switch (currentWiFiItem) {
                    case WIFI_SCAN:
                        startNetworkScan();
                        Serial.println("[MENU] Network scan started");
                        break;
                    case WIFI_DEAUTH:
                        if (networkCount > 0) {
                            startDeauthAttack(networks[selectedItem % networkCount].bssid, 
                                            networks[selectedItem % networkCount].channel);
                            Serial.println("[MENU] Deauth attack started");
                        }
                        break;
                    case WIFI_BEACON:
                        startBeaconSpoof("TestSSID", 6);
                        Serial.println("[MENU] Beacon spoof started");
                        break;
                    case WIFI_STATS:
                        enablePacketSniffer(6);
                        Serial.println("[MENU] Packet sniffer enabled");
                        break;
                    case WIFI_BACK:
                        setCurrentMenu(MENU_HOME);
                        currentHomeItem = HOME_WIFI;
                        break;
                    default:
                        break;
                }
                break;
                
            case MENU_SETTINGS_SUBMENU:
                switch (currentSettingsItem) {
                    case SETTINGS_CHANNEL:
                        Serial.println("[MENU] Channel settings");
                        break;
                    case SETTINGS_POWER:
                        Serial.println("[MENU] Power settings");
                        break;
                    case SETTINGS_BRIGHTNESS:
                        Serial.println("[MENU] Brightness settings");
                        break;
                    case SETTINGS_BACK:
                        setCurrentMenu(MENU_HOME);
                        currentHomeItem = HOME_SETTINGS;
                        break;
                    default:
                        break;
                }
                break;
                
            case MENU_INFO_SUBMENU:
                switch (currentInfoItem) {
                    case INFO_DEVICE:
                        Serial.println("[MENU] Device info");
                        break;
                    case INFO_STATUS:
                        Serial.println("[MENU] Status info");
                        break;
                    case INFO_BACK:
                        setCurrentMenu(MENU_HOME);
                        currentHomeItem = HOME_INFO;
                        break;
                    default:
                        break;
                }
                break;
                
            default:
                break;
        }
    }
    
    // Navigation via up/down (with debouncing)
    if (y < -1500) { // Up
        if ((currentTime - lastUpButtonTime) >= NAV_DEBOUNCE_MS) {
            if (currentMenu == MENU_HOME) {
                currentHomeItem = (HomeMenuItem)((currentHomeItem - 1 + HOME_SUBMENU_COUNT) % HOME_SUBMENU_COUNT);
            } else if (currentMenu == MENU_WIFI_SUBMENU) {
                currentWiFiItem = (WiFiSubmenuItem)((currentWiFiItem - 1 + WIFI_SUBMENU_COUNT) % WIFI_SUBMENU_COUNT);
            } else if (currentMenu == MENU_SETTINGS_SUBMENU) {
                currentSettingsItem = (SettingsSubmenuItem)((currentSettingsItem - 1 + SETTINGS_SUBMENU_COUNT) % SETTINGS_SUBMENU_COUNT);
            } else if (currentMenu == MENU_INFO_SUBMENU) {
                currentInfoItem = (InfoSubmenuItem)((currentInfoItem - 1 + INFO_SUBMENU_COUNT) % INFO_SUBMENU_COUNT);
            }
            lastUpButtonTime = currentTime;
        }
    } else if (y > 1500) { // Down
        if ((currentTime - lastDownButtonTime) >= NAV_DEBOUNCE_MS) {
            if (currentMenu == MENU_HOME) {
                currentHomeItem = (HomeMenuItem)((currentHomeItem + 1) % HOME_SUBMENU_COUNT);
            } else if (currentMenu == MENU_WIFI_SUBMENU) {
                currentWiFiItem = (WiFiSubmenuItem)((currentWiFiItem + 1) % WIFI_SUBMENU_COUNT);
            } else if (currentMenu == MENU_SETTINGS_SUBMENU) {
                currentSettingsItem = (SettingsSubmenuItem)((currentSettingsItem + 1) % SETTINGS_SUBMENU_COUNT);
            } else if (currentMenu == MENU_INFO_SUBMENU) {
                currentInfoItem = (InfoSubmenuItem)((currentInfoItem + 1) % INFO_SUBMENU_COUNT);
            }
            lastDownButtonTime = currentTime;
        }
    }
    
    // Left navigation (independent)
    if (x < -2000 && (currentTime - lastLeftButtonTime) >= NAV_DEBOUNCE_MS) {
        Serial.println("[MENU] Left direction pressed");
        lastLeftButtonTime = currentTime;
    }
    
    // Right navigation (independent)
    if (x > 2000 && (currentTime - lastRightButtonTime) >= NAV_DEBOUNCE_MS) {
        Serial.println("[MENU] Right direction pressed");
        lastRightButtonTime = currentTime;
    }
}

void drawCurrentMenu() {
    switch (currentMenu) {
        case MENU_HOME:
            drawHomeMenu();
            break;
            
        case MENU_WIFI_SUBMENU:
            drawWiFiSubmenu();
            break;
            
        case MENU_SETTINGS_SUBMENU:
            drawSettingsMenu();
            break;
            
        case MENU_INFO_SUBMENU:
            drawInfoMenu();
            break;
            
        default:
            break;
    }
}

MenuScreen getCurrentMenu() {
    return currentMenu;
}

void setCurrentMenu(MenuScreen menu) {
    if (menu >= MENU_COUNT) return;
    currentMenu = menu;
    Serial.printf("[MENU] Menu changed to: %d\n", menu);
}

HomeMenuItem getCurrentHomeItem() {
    return currentHomeItem;
}

void setCurrentHomeItem(HomeMenuItem item) {
    if (item >= HOME_SUBMENU_COUNT) return;
    currentHomeItem = item;
    Serial.printf("[MENU] Home menu item changed to: %d\n", item);
}

WiFiSubmenuItem getCurrentWiFiItem() {
    return currentWiFiItem;
}

void setCurrentWiFiItem(WiFiSubmenuItem item) {
    if (item >= WIFI_SUBMENU_COUNT) return;
    currentWiFiItem = item;
    Serial.printf("[MENU] WiFi submenu item changed to: %d\n", item);
}

SettingsSubmenuItem getCurrentSettingsItem() {
    return currentSettingsItem;
}

void setCurrentSettingsItem(SettingsSubmenuItem item) {
    if (item >= SETTINGS_SUBMENU_COUNT) return;
    currentSettingsItem = item;
    Serial.printf("[MENU] Settings submenu item changed to: %d\n", item);
}

InfoSubmenuItem getCurrentInfoItem() {
    return currentInfoItem;
}

void setCurrentInfoItem(InfoSubmenuItem item) {
    if (item >= INFO_SUBMENU_COUNT) return;
    currentInfoItem = item;
    Serial.printf("[MENU] Info submenu item changed to: %d\n", item);
}

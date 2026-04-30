#pragma once

// Main menu screens
typedef enum {
    MENU_HOME = 0,
    MENU_WIFI_SUBMENU,
    MENU_SETTINGS_SUBMENU,
    MENU_INFO_SUBMENU,
    MENU_COUNT
} MenuScreen;

// WiFi submenu items
typedef enum {
    WIFI_SCAN = 0,
    WIFI_DEAUTH,
    WIFI_BEACON,
    WIFI_STATS,
    WIFI_BACK,
    WIFI_SUBMENU_COUNT
} WiFiSubmenuItem;

// Settings submenu items
typedef enum {
    SETTINGS_CHANNEL = 0,
    SETTINGS_POWER,
    SETTINGS_BRIGHTNESS,
    SETTINGS_BACK,
    SETTINGS_SUBMENU_COUNT
} SettingsSubmenuItem;

// Info submenu items
typedef enum {
    INFO_DEVICE = 0,
    INFO_STATUS,
    INFO_BACK,
    INFO_SUBMENU_COUNT
} InfoSubmenuItem;

// Home menu items
typedef enum {
    HOME_WIFI = 0,
    HOME_SETTINGS,
    HOME_INFO,
    HOME_SUBMENU_COUNT
} HomeMenuItem;

void initMenuSystem();
void handleMenuInput(int x, int y, bool button_pressed);
void drawCurrentMenu();
MenuScreen getCurrentMenu();
void setCurrentMenu(MenuScreen menu);
HomeMenuItem getCurrentHomeItem();
void setCurrentHomeItem(HomeMenuItem item);
WiFiSubmenuItem getCurrentWiFiItem();
void setCurrentWiFiItem(WiFiSubmenuItem item);
SettingsSubmenuItem getCurrentSettingsItem();
void setCurrentSettingsItem(SettingsSubmenuItem item);
InfoSubmenuItem getCurrentInfoItem();
void setCurrentInfoItem(InfoSubmenuItem item);

# Project WindBreaker


# Adding secrets:
In order for the device to connect to the internet you need to add a ``` secrets.h``` file.
Create a new "secrets.h' file. This is where secrets will be stored

``` 
#define WIFI_SSID 
#define WIFI_PASS ""
```

# Device Mapping

✅ VCC → 3V3 (NOT 5V)
✅ GND → GND
✅ D0 → GPIO18
✅ D1 → GPIO23
✅ CS → GPIO14
✅ DC → GPIO26
✅ RES → GPIO25

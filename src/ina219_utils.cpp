#include <Adafruit_INA219.h>
#include "ina219_utils.h"
#include "display.h"
#include <DHT.h>
#include <DHT_U.h>

#define DHT11_PIN 2
#define DHTTYPE DHT11

namespace INA219_utils {

    Adafruit_INA219 ina219;
    DHT_Unified dht(DHT11_PIN, DHTTYPE);

    void init() {
        if (!ina219.begin()) {
            displayShow(emptyString, "Failed to find INA219 chip", emptyString,5000);
            while (1) { 
                delay(10); 
            }
        }
        ina219.setCalibration_32V_1A();
        dht.begin();
    }

    float  getLoadVoltage() {
        float shuntvoltage = ina219.getShuntVoltage_mV();
        float busvoltage = ina219.getBusVoltage_V();
        float loadvoltage = busvoltage + (shuntvoltage / 1000);
        return loadvoltage;
    }

    float getHumidity() {
        sensors_event_t event;
        dht.humidity().getEvent(&event);
        if (isnan(event.relative_humidity)) {
            return -100;
        }
        return event.relative_humidity;
    }

    float getTemperature() {
        sensors_event_t event;
        dht.temperature().getEvent(&event);
        if (isnan(event.temperature)) {
            return -100;
        }
        return event.temperature;
    }

}
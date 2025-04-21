#ifndef BATTERY_UTILS_H_
#define BATTERY_UTILS_H_

#include <Arduino.h>


namespace BATTERY_Utils {

    String  generateEncodedTelemetry(float batVoltage, float accVoltage, float temp, float hum);
    String  getPercentVoltageBattery(float voltage);
    void    checkVoltageWithoutGPSFix();
    
}

#endif
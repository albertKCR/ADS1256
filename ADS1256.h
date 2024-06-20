#ifndef ADS1256_H
#define ADS1256_H

#include "ADS_DATA.h"
#include "Arduino.h"
#include <SPI.h>

class ADS1256 {

#define ADS_RST_PIN 17 // ADS1256 reset pin
#define ADS_RDY_PIN 16 // ADS1256 data ready
#define ADS_CS_PIN 5   // ADS1256 chip select

    bool DRDY_state;

public:
    float OutputVoltage; // Stores read voltage
    float OutputCurrent; // Stores read current

    double voltageRead;
    double resolution; // 2^23 - 1

    // This needs to match the setting in the ADC init function in the library tab
    double Gain; // Be sure to have a period

    double vRef; // Reference voltage

    // We'll calculate this in setup
    double bitToVolt;

    ADS1256();
    void initialize();
    void readChannelData(int channel);
    void waitForDataReady();
    void handleDRDYInterrupt();
    void resetADC();
    void sendCommand(uint8_t cmd);
    double getAverageReading(int numSamples, int channel);
    float getVoltage();
    float getCurrent();
    unsigned long readRegisterValue(uint8_t registerAddress);
    void writeRegisterValue(uint8_t registerAddress, uint8_t registerValue);
};

#endif

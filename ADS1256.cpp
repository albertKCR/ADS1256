#include "ADS1256.h"
#include "ADS_DATA.h"

// Constructor to initialize constants
ADS1256::ADS1256() {
    resolution = 8388608.0; // 2^23 - 1, max ADC resolution
    Gain = 1.0; // Gain setting for ADC
    vRef = 5.0478; // Reference voltage
    bitToVolt = resolution * Gain / vRef; // Conversion factor from bits to voltage

    DRDY_state = HIGH; // Initialize DRDY state
}

// Initialize the ADS1256 ADC
void ADS1256::initialize() {
    SPI.begin(); // Initialize SPI communication

    // Set pin modes
    pinMode(ADS_CS_PIN, OUTPUT);
    pinMode(ADS_RDY_PIN, INPUT);
    pinMode(ADS_RST_PIN, OUTPUT);

    // Perform hardware reset
    digitalWrite(ADS_RST_PIN, LOW);
    delay(10);
    digitalWrite(ADS_RST_PIN, HIGH);
    delay(1000);

    // Perform ADS reset
    resetADC();
    delay(2000);

    // Setup ADS registers with initial settings
    writeRegisterValue(MUX, MUX_RESET); // Reset MUX register
    writeRegisterValue(ADCON, PGA_1);   // Set gain to 1
    writeRegisterValue(DRATE, DR_100);  // Set data rate to 100SPS
    delay(2000);

    // Perform self-calibration
    sendCommand(SELFCAL);
    delay(5);
}

// Return the current output current value
float ADS1256::getCurrent() {
    return OutputCurrent; // Return the stored current value
}

// Return the current output voltage value
float ADS1256::getVoltage() {
    return OutputVoltage; // Return the stored voltage value
}

// Average multiple readings from a specified channel
double ADS1256::getAverageReading(int numSamples, int channel) {
    double total = 0.0;
    for (int i = 0; i < numSamples; i++) {
        readChannelData(channel); // Read data from the specified channel
        total += voltageRead;     // Accumulate the readings
    }
    voltageRead = total / numSamples; // Calculate the average
    return voltageRead; // Return the averaged reading
}

// Interrupt service routine for data ready signal
void ADS1256::handleDRDYInterrupt() {
    DRDY_state = LOW; // Set DRDY state to LOW when interrupt occurs
}

// Perform a software reset of the ADS1256
void ADS1256::resetADC() {
    SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE1));
    digitalWrite(ADS_CS_PIN, LOW); // Select the ADC
    delayMicroseconds(10);
    SPI.transfer(RESET);  // Send reset command
    delay(2);
    SPI.transfer(SDATAC); // Send stop data continuous mode command
    delayMicroseconds(100);
    digitalWrite(ADS_CS_PIN, HIGH); // Deselect the ADC
    SPI.endTransaction();
}

// Send a command to the ADS1256
void ADS1256::sendCommand(uint8_t cmd) {
    waitForDataReady(); // Wait until the data is ready
    SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE1));
    digitalWrite(ADS_CS_PIN, LOW); // Select the ADC
    delayMicroseconds(10);
    SPI.transfer(cmd); // Send the command
    delayMicroseconds(10);
    digitalWrite(ADS_CS_PIN, HIGH); // Deselect the ADC
    SPI.endTransaction();
}

// Wait for the DRDY pin to go LOW indicating data is ready
void ADS1256::waitForDataReady() {
    while (DRDY_state) {
        // Busy-wait loop until DRDY goes LOW
    }
    DRDY_state = HIGH; // Reset DRDY state
}

// Read data from a specified channel
void ADS1256::readChannelData(int channel) {
    int32_t adcValue = 0; // Variable to store ADC value

    waitForDataReady(); // Wait until data is ready
    SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE1));
    digitalWrite(ADS_CS_PIN, LOW); // Select the ADC
    delayMicroseconds(5);

    // Set MUX register to the desired channel
    SPI.transfer(WREG | MUX);
    SPI.transfer(0x00);
    SPI.transfer(channel);

    delayMicroseconds(5);
    SPI.transfer(SYNC); // Synchronize
    delayMicroseconds(5);
    SPI.transfer(WAKEUP); // Wake up ADC
    delayMicroseconds(1);
    SPI.transfer(RDATA); // Request data
    delayMicroseconds(7);

    // Read ADC data (24 bits)
    adcValue |= SPI.transfer(NOP);
    adcValue <<= 8;
    adcValue |= SPI.transfer(NOP);
    adcValue <<= 8;
    adcValue |= SPI.transfer(NOP);

    digitalWrite(ADS_CS_PIN, HIGH); // Deselect the ADC
    SPI.endTransaction();

    // Convert ADC value to voltage
    if (adcValue > 0x7FFFFF) {
        adcValue -= 16777216; // Handle negative values
    }
    voltageRead = static_cast<double>(adcValue) / bitToVolt; // Convert to voltage
}

// Read a value from a specified register
unsigned long ADS1256::readRegisterValue(uint8_t registerAddress) {
    uint8_t registerValue = 0;

    waitForDataReady(); // Wait until data is ready
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    digitalWrite(ADS_CS_PIN, LOW); // Select the ADC
    delayMicroseconds(7);

    SPI.transfer(0x10 | registerAddress); // Send read register command
    SPI.transfer(0x00);
    delayMicroseconds(7);
    registerValue = SPI.transfer(0xFF); // Read register value

    delayMicroseconds(7);
    digitalWrite(ADS_CS_PIN, HIGH); // Deselect the ADC
    SPI.endTransaction();

    return registerValue; // Return the register value
}

// Write a value to a specified register
void ADS1256::writeRegisterValue(uint8_t registerAddress, uint8_t registerValue) {
    waitForDataReady(); // Wait until data is ready
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    digitalWrite(ADS_CS_PIN, LOW); // Select the ADC
    delayMicroseconds(7);

    SPI.transfer(0x50 | registerAddress); // Send write register command
    SPI.transfer(0x00);
    delayMicroseconds(7);
    SPI.transfer(registerValue); // Write register value

    delayMicroseconds(7);
    digitalWrite(ADS_CS_PIN, HIGH); // Deselect the ADC
    SPI.endTransaction();
}

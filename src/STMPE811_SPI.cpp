#include "STMPE811_SPI.h"

//#include <logging.h>
//extern uLog theLog;

STMPE811::STMPE811(uint8_t theChipSelect) : chipSelect{theChipSelect} {
}

void STMPE811::initialize()        // Based on AN2807, but adapted for polling io interrupt driven
{
//    theLog.output(loggingLevel::Info, "STMPE811::initialize()");

    pinMode(chipSelect, OUTPUT);           // set chipSelect as output from MCU to STMPE
    digitalWrite(chipSelect, HIGH);        // disable chipSelect - active low signal

    // TODO : workaround for potential HW problem : Float inputs on the chip making it undefined which SPI mode it will use
    for (theSpiMode = 0; theSpiMode < 4; theSpiMode++) {        // Try all SPI modes until the version reads a proper 0x811 value..  Is a workaround for the floating pins which may result in wrong SPI mode
        write8(SYS_CTRL1, 0b00000010);                          // Reset the STMPE811
        write8(SYS_CTRL2, 0b00000100);                          // turn on clock for ADC , TSC and Temp Sensor. GPIO still off for the time being
        if (0x811 == getVersion()) {                            // Check if this works by trying to read the version register, should be 0x811 - magical value
            break;
        }
    }

    if (0x811 != getVersion()) {
//        theLog.output(loggingLevel::Error, "Could not Initialize STMPE811");
    } else {
//        theLog.snprintf(loggingLevel::Debug, "STMPE811 Version = 0x%04X, using SPI Mode %d", getVersion(), theSpiMode);
    }

    write8(ADC_CTRL1, 0b01001000);             // ADC conversion time in 80 clock ticks, 12-bit resolution, internal reference
    delay(2);                                  // as mentioned in AN2807
    write8(ADC_CTRL2, 0b00000001);             // 3.25 MHz ADC clock speed
    write8(GPIO_ALT_FUNCT, 0b00000000);        // See AN2807 1. $5

    write8(TSC_CFG, 0b10101100);        // Average control : 4 samples,  Touch detect delay : 5ms, Paneldriver settling time 5 ms

    write8(TSC_CTRL, 0b00000011);           // Tracking index No window tracking, TSC operating mode : X, Y acquisition, Enable TSC
    write8(TSC_I_DRIVE, 0b00000001);        // maximum current on the touchscreen controller (TSC) driving channel : 50 mA typical, 80 mA max
    write8(TSC_FRACT_Z, 0x7);               // This register allows to select the range and accuracy of the pressure measurement. AN2807 : "recommended value (7 fractional part and 1 whole part)"

    write8(INT_EN, 0x0);        // No interrupts

    write8(FIFO_CTRL_STA, 0b00000001);        // reset the FIFO
    write8(FIFO_CTRL_STA, 0x00);              // AN2807 : "Write 0x00 to put the FIFO back into operation mode."
}

boolean STMPE811::isTouched() {
    readFifo();
    return (read8(TSC_CTRL) & TSC_STA_MASK);
}

int16_t STMPE811::getX() {
    // TODO : whenever getX, getY is isTouch is called, read all data from the FIFO to empty it and update lastX, lastY etc
    // then return those last values
    readFifo();
    return lastX;
}

int16_t STMPE811::getY() {
    readFifo();
    return lastY;
}

uint16_t STMPE811::getVersion() {
    uint16_t version = (read8(CHIP_ID) << 8) | read8(CHIP_ID + 1);
    return version;
}

uint8_t STMPE811::getRevision() {
    return read8(ID_VER);
}

void STMPE811::readFifo() {
    uint8_t fifoLevel = read8(FIFO_SIZE);
    for (uint8_t i = 1; i < fifoLevel; i++) {
        // if there is more than 1 position in the FIFO, we read and discard all the older ones
        (void)read8(TSC_DATA);
        (void)read8(TSC_DATA);
        (void)read8(TSC_DATA);
    }

    if (fifoLevel >= 1) {
        uint8_t rawData[3];
        rawData[0] = read8(TSC_DATA);
        rawData[1] = read8(TSC_DATA);
        rawData[2] = read8(TSC_DATA);

        lastX = (rawData[0] << 4) + ((rawData[1] >> 4) & 0x0F);
        lastY = ((rawData[1] & 0x0F) << 8) + rawData[2];
    }

    write8(FIFO_CTRL_STA, 0b00000001);        // reset the FIFO
    write8(FIFO_CTRL_STA, 0x00);              // AN2807 : "Write 0x00 to put the FIFO back into operation mode."
}

uint8_t STMPE811::read8(uint8_t address)        // See specification section 5.1.1
{
    uint8_t data = 0;                                                        // stores the result of the read operation
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, theSpiMode));        // set the SPI in the correct mode for STMPE
    digitalWrite(chipSelect, LOW);                                           // Enable chipSelect so STMPE will listen to SPI
    SPI.transfer(address | 0x80);                                            // write address byte + MSB==1 to indicate read operation
    SPI.transfer(0x00);                                                      // ???
    data = SPI.transfer(0);                                                  // read byte from SPI
    digitalWrite(chipSelect, HIGH);                                          // disable STMPE, disconnects from SPI
    SPI.endTransaction();                                                    // stop SPI transmission
    return data;
}

void STMPE811::write8(uint8_t address, uint8_t data)        // See specification section 5.1.2
{
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, theSpiMode));        // set the SPI in the correct mode for STMPE
    digitalWrite(chipSelect, LOW);                                           // Enable chipSelect so STMPE will listen to SPI
    SPI.transfer(address & 0x7F);                                            // write address byte + MSB==0 to indicate a write operation
    SPI.transfer(data);                                                      // write data byte
    digitalWrite(chipSelect, HIGH);                                          // disable STMPE, disconnects from SPI
    SPI.endTransaction();                                                    // stop SPI transmission
}

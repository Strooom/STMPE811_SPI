// #############################################################################
// ###                                                                       ###
// ### Arduino style driver for STMPE811 Touch Controller                    ###
// ### https://github.com/Strooom/STMPE811_SPI                               ###
// ### Author(s) : Pascal Roobrouck - @strooom                               ###
// ### License : https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode ###
// ###                                                                       ###
// #############################################################################

// Only supports the hardware SPI interface of the MCU - developed for ESP32 using the built-in hardware of the ESP32
// This driver was developed to drive an STMPE811 as TouchScreen controller. As such the other applications such as IO expander, ADC and temperature sensor are to be added.

#pragma once

#ifndef WIN32
#include <Arduino.h>
#include <SPI.h>
#include "STMP811registers.h"
#else
#include <inttypes.h>
#include <cstring>
#endif

class STMPE811 {
  public:
    explicit STMPE811(uint8_t chipSelect);        // Constructor
    bool initialize();                            // initialize the STMPE811
    uint16_t getVersion();                        // reads the CHIP_ID register from the device and should always read 0x0811
    uint8_t getRevision();                        // reads the ID_VER register from the device
    boolean isTouched();                          // returns true if screen is currently touched
    int16_t getX();                               // returns last x-coordinate touched
    int16_t getY();                               // returns last y-coordinate touched

#ifndef UnitTesting
  private:        // commented out during unit testing
#endif
    uint8_t theSpiMode{0};        // depending on how the chip is wired, it can use different spi mode
    uint8_t chipSelect;           // active low chipSelect to enable the STMPE's SPI communication
    int16_t lastX{0};
    int16_t lastY{0};

    void write8(uint8_t address, uint8_t data);        // write a byte 'data' to the STMPE811 register with 'address'
    uint8_t read8(uint8_t address);                    // read a byte from STMPE811 register with 'address'

    void readFifo();

    // STMPE811 Control Register Addresses,					// see specification table 10
    // System and identification registers					// see specification table 11
    static constexpr uint8_t CHIP_ID   = 0x00;        // Device identification, read-only, reads 0x0811
    static constexpr uint8_t ID_VER    = 0x02;        // Revision number
    static constexpr uint8_t SYS_CTRL1 = 0x03;        // Reset control
    static constexpr uint8_t SYS_CTRL2 = 0x04;        // Clock control
    static constexpr uint8_t SPI_CFG   = 0x08;        // SPI interface configuration

    // Interrupt system
    static constexpr uint8_t INT_CTRL     = 0x09;        // Interrupt control register
    static constexpr uint8_t INT_EN       = 0x0A;        // Interrupt enable register
    static constexpr uint8_t INT_STA      = 0x0B;        // interrupt status register
    static constexpr uint8_t GPIO_EN      = 0x0C;        // GPIO interrupt enable
    static constexpr uint8_t GPIO_INT_STA = 0x0D;        // GPIO interrupt status

    // Analog-to-digital converter							// see specification table 12
    static constexpr uint8_t ADC_CTRL1    = 0x20;        // ADC control
    static constexpr uint8_t ADC_CTRL2    = 0x21;        // ADC control
    static constexpr uint8_t ADC_CAPT     = 0x22;        // To initiate ADC Data acquisition
    static constexpr uint8_t ADC_DATA_CH0 = 0x30;        // ADC channel 0 (IN3/GPIO-3)
    static constexpr uint8_t ADC_DATA_CH1 = 0x32;        // ADC channel 1 (IN2/GPIO-2)
    static constexpr uint8_t ADC_DATA_CH2 = 0x34;        // ADC channel 2 (IN1/GPIO-1)
    static constexpr uint8_t ADC_DATA_CH3 = 0x36;        // ADC channel 3 (IN0-GPIO-0)
    static constexpr uint8_t ADC_DATA_CH4 = 0x38;        // ADC channel 4 (TSC)
    static constexpr uint8_t ADC_DATA_CH5 = 0x3A;        // ADC channel 5 (TSC)
    static constexpr uint8_t ADC_DATA_CH6 = 0x3C;        // ADC channel 6 (TSC)
    static constexpr uint8_t ADC_DATA_CH7 = 0x3E;        // ADC channel 7 (TSC)

    // Touchscreen controller								// see specification table 14
    static constexpr uint8_t TSC_CTRL = 0x40;        // 4-wire touchscreen controller setup
    static constexpr uint8_t TSC_CFG  = 0x41;        // Touchscreen controller configuration

    static constexpr uint8_t WDW_TR_X      = 0x42;        //	Window setup for top right X
    static constexpr uint8_t WDW_TR_Y      = 0x44;        //	Window setup for top right Y
    static constexpr uint8_t WDW_BL_X      = 0x46;        //	Window setup for bottom left X
    static constexpr uint8_t WDW_BL_Y      = 0x48;        //	Window setup for bottom left Y
    static constexpr uint8_t FIFO_TH       = 0x4A;        //	FIFO level to generate interrupt
    static constexpr uint8_t FIFO_CTRL_STA = 0x4B;        //	Current status of FIFO
    static constexpr uint8_t FIFO_SIZE     = 0x4C;        //	Current filled level of FIFO
    static constexpr uint8_t TSC_DATA_X    = 0x4D;        //	Data port for TSC data access
    static constexpr uint8_t TSC_DATA_Y    = 0x4F;        //	Data port for TSC data access
    static constexpr uint8_t TSC_DATA_Z    = 0x51;        //	Data port for TSC data access
    static constexpr uint8_t TSC_DATA_XYZ  = 0x52;        //	Data port for TSC data access
    static constexpr uint8_t TSC_FRACT_Z   = 0x56;        //
    static constexpr uint8_t TSC_DATA      = 0x57;        //	TSC data access port
    static constexpr uint8_t TSC_I_DRIVE   = 0x58;        //
    static constexpr uint8_t TSC_SHIELD    = 0x59;        //

    // Temperature sensor									// see specification table 16
    static constexpr uint8_t TEMP_CTRL = 0x60;        // Temperature sensor setup
    static constexpr uint8_t TEMP_DATA = 0x61;        // Temperature data access port
    static constexpr uint8_t TEMP_TH   = 0x62;        // Threshold for temperature controlled interrupt

    // GPIO controller										// see specification table 17
    static constexpr uint8_t GPIO_SET_PIN   = 0x10;        // Set pin register
    static constexpr uint8_t GPIO_CLR_PIN   = 0x11;        // Clear pin state
    static constexpr uint8_t GPIO_MP_STA    = 0x12;        // Monitor pin state
    static constexpr uint8_t GPIO_DIR       = 0x13;        // Set pin direction
    static constexpr uint8_t GPIO_ED        = 0x14;        // Edge	detect status
    static constexpr uint8_t GPIO_RE        = 0x15;        // Rising edge
    static constexpr uint8_t GPIO_FE        = 0x16;        // Falling edge
    static constexpr uint8_t GPIO_ALT_FUNCT = 0x17;        // Alternate function register

    // STMPE811 Control Register Values
    static constexpr uint8_t TSC_STA_MASK   = 0x80;        // Mask to select only the TS_STA bit
    static constexpr uint8_t SYS_CLOCK_TS   = 0x08;        // Clock Control for Temperature Sensor
    static constexpr uint8_t SYS_CLOCK_GPIO = 0x04;        // Clock Control for General Purpose IO
    static constexpr uint8_t SYS_CLOCK_TSC  = 0x02;        // Clock Control for Touch Screen Controller
    static constexpr uint8_t SYS_CLOCK_ADC  = 0x01;        // Clock Control for Analog2Digital Convertor
};

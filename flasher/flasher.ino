/**
 * @file flasher.ino
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-12-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Arduino.h"

/**
 * @brief Pins used for the D-pad on the PCB
 * 
 */
const byte UP = 14;
const byte RIGHT = 9;

const byte LEFT = 10;
const byte DOWN = 11;
const byte ENTER = 8;

/**
 * @brief Constants used for the display
 * 
 */
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/**
 * @brief Constants used for communicaton with the SD card
 * 
 */
//NONE

/**
 * @brief Constants used for communication with target device
 * 
 */
#include "Adafruit_DAP.h"

#define SD_CS 4
#define SWDIO 12
#define SWCLK 11
#define SWRST 9
const int BUFSIZE = Adafruit_DAP_SAM::PAGESIZE;
uint8_t buf[BUFSIZE];

//create a DAP for programming Atmel SAM devices
Adafruit_DAP_SAM dap;

// Function called when there's an SWD error
void error(const char *text) {
  Serial.println(text);
  while (1);
}


void setup(){

    /**
     * @brief Setup communication and interrupts for 
     * d-pad
     * 
     */
    pinMode(ENTER, INPUT_PULLDOWN);
    pinMode(RIGHT, INPUT_PULLDOWN);
    pinMode(LEFT, INPUT_PULLDOWN);
    pinMode(UP, INPUT_PULLDOWN);
    pinMode(DOWN, INPUT_PULLDOWN);

        //to enable the interrupt on PA8
    PORT->Group[PORTA].PINCFG[8].bit.INEN = 1;        // Enable the (optional) pull-up resistor on PA08
    PORT->Group[PORTA].PINCFG[8].bit.PULLEN = 1;      
    PORT->Group[PORTA].PINCFG[8].bit.PMUXEN = 1;      // Enable the port multiplexer on PA08
    PORT->Group[PORTA].PMUX[8 >> 1].reg |= PORT_PMUX_PMUXE_A; // Switch the port multiplexer to EIC on PA08
    
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_EIC |         // Disconnect GCLK0 from the EIC
                        GCLK_CLKCTRL_GEN_GCLK0;       // Select GCLK0                                          
    while (GCLK->STATUS.bit.SYNCBUSY);                // Wait for synchronization

    EIC->NMICTRL.reg = EIC_NMICTRL_NMISENSE_RISE;     // Trigger an NMI interrupt on the a rising edge
    // EIC->NMICTRL.reg |= EIC_NMICTRL_NMIFILTEN;       // Enable NMI filter

    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |          // Enable GCLK
                        GCLK_CLKCTRL_ID_EIC |         // Connect GCLK0 to the EIC
                        GCLK_CLKCTRL_GEN_GCLK0;       // Select GCLK0                                          
    while (GCLK->STATUS.bit.SYNCBUSY);  
    attachInterrupt(digitalPinToInterrupt(UP), up, RISING);
    attachInterrupt(digitalPinToInterrupt(DOWN), down_isr, RISING);
    attachInterrupt(digitalPinToInterrupt(RIGHT), right, RISING);
    attachInterrupt(digitalPinToInterrupt(LEFT), left, RISING);
    EIC->CONFIG[1].reg |= EIC_CONFIG_SENSE2_RISE;  

    /**
     * @brief Setup communication with the OLED display
     * 
     */
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    } 
    display.display();
    delay(2000); // Pause for 2 seconds

    // Clear the buffer
    display.clearDisplay();

    // Draw a single pixel in white
    display.drawPixel(10, 10, SSD1306_WHITE);

    // Show the display buffer on the screen. You MUST call display() after
    // drawing commands to make them visible on screen!
    display.display();

    /**
     * @brief Setup communication with the SD card
     * 
     */
    if (!SD.begin(23)) {
        Serial.println("initialization failed!");
        while (1);
    }

    /**
     * @brief Setup for communication with target device
     * 
     */
    dap.begin(SWCLK, SWDIO, SWRST, &error);

    /**
     * @brief Code to select a file on the SD card
     * 
     */


    /**
     * @brief Code to attempt to flash the target device
     * 
     */
}

void loop(){

}
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
// #include <SD.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Arduino.h"
#include "Adafruit_DAP.h"
#include <SPI.h>
#include <SdFat.h>

/**
 * @brief Pins used for the D-pad on the PCB
 * 
 */
const byte UP = 14;
const byte RIGHT = 9;

// const byte LEFT = 10;
const byte LEFT = 28;
// const byte DOWN = 11;
const byte DOWN = 27;
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

#define SD_CS 23
#define SWDIO 6
#define SWCLK 7
#define SWRST 4
const int BUFSIZE = Adafruit_DAP_SAM::PAGESIZE;
uint8_t buf[BUFSIZE];

//create a DAP for programming Atmel SAM devices
Adafruit_DAP_SAM dap;
SdFat SD;

// Function called when there's an SWD error
void error(const char *text) {
  display.println(text);
  display.display();
  while (1);
}


//setup file tree display
File root;
File currentFile;

struct file_entry{
  File loc;
  char name[15];
  bool folder;
  bool opened;
};

#define DSIZE 100
file_entry start[DSIZE]; 
int current_display;


bool enter_pressed = false;
bool right_pressed = false;
bool left_pressed = false;
bool down_pressed = false;
bool up_pressed = false;

void setup(){
    Serial.begin(115200);
    // while(!Serial){}
    /**
     * @brief Setup communication and interrupts for 
     * d-pad
     * 
     */
    pinMode(5, OUTPUT);
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
    /**
     * @brief Setup communication with the SD card
     * 
     */
    if (!SD.begin(SD_CS)) {
        Serial.println("initialization failed!");
        while (1);
    }
    root = SD.open("/binaries");
    if(!root) {
        Serial.println("Failed to open SD root directory!");
        return;
    }
    printDirectory(root, 0);
    // currentFile = root;



    /**
     * @brief Code to select a file on the SD card
     * 
     */
    // display.clearDisplay();
    // display.setCursor(0, 0);
    // currentFile = root;
    // printFileTree(currentFile);
    display.clearDisplay();   
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("SD Card Files:");
    display.display();
    current_display = 0;
    printFileTree();
    delay(2000);
    // printDirectory(root, 0);

    // up_pressed = false;
    while(!enter_pressed){
      if (up_pressed == true) {
        if(current_display > 0){
          current_display--;
          printFileTree();
        }
        up_pressed = false;
      }

      if (down_pressed == true) {
        if(current_display < DSIZE){
          current_display++;
          printFileTree();
        }
        down_pressed = false;
      }
      if(right_pressed == true){
        right_pressed = false;
      }
    } //wait until the enter button is pressed with desired file. 
    enter_pressed = false;

    /**
     * @brief Setup for communication with target device
     * 
     */
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Press Enter to flash: ");
    display.println(start[current_display].name);
    display.display();
    while(enter_pressed == false){
      delay(10);
    }
    enter_pressed = false;
    dap.begin(SWCLK, SWDIO, SWRST, &error);
    /**
     * @brief Code to attempt to flash the target device
     * 
     */
    File32 dataFile = start[current_display].loc;
      display.clearDisplay();
      display.setCursor(0,0);
      // display.display();
      if(!dataFile){
        error("Couldn't open file");
      }
      
      // display.println("Connecting...");
      if ( !dap.targetConnect() ) {
        error(dap.error_message);
      }

      char debuggername[100];
      dap.dap_get_debugger_info(debuggername);
      // display.print(debuggername); display.print("\n\r");

      uint32_t dsu_did;
      if (! dap.select(&dsu_did)) {
        // display.print("Unknown device found 0x"); display.print(dsu_did, HEX);
        error("Unknown device found");
      }
      // display.clearDisplay();
      display.print("Found: ");
      display.println(dap.target_device.name);
      display.print("\tSize: ");
      display.println(dap.target_device.flash_size);
      // display.print("\tFlash pages: ");
      // display.println(dap.target_device.n_pages);
      //Serial.print("Page size: "); Serial.println(dap.target_device.flash_size / dap.target_device.n_pages);
      
      /* Example of how to read and set fuses
      Serial.print("Fuses... ");
      dap.fuseRead(); //MUST READ FUSES BEFORE SETTING OR WRITING ANY
      dap._USER_ROW.bit.WDT_Period = 0x0A;
      dap.fuseWrite();
      Serial.println(" done.");
      */
      display.println("Press enter...");
      display.display();
      while(enter_pressed == false){
        delay(10);
      }
      display.clearDisplay();
      display.setCursor(0,0);
      display.print("Erasing... ");
      dap.erase();
      display.println(" done.");
      
      display.print("Programming... ");
      unsigned long t = millis();
      uint32_t addr = dap.program_start();

      while (dataFile.available()) {
          memset(buf, BUFSIZE, 0xFF);  // empty it out
          dataFile.read(buf, BUFSIZE);
          dap.programBlock(addr, buf);
          addr += BUFSIZE;
      }
      dataFile.close();
      display.println(millis() - t);
      display.println("\nDone!");
      display.display();
      dap.dap_set_clock(50);

      dap.deselect();
      dap.dap_disconnect();
}

void loop(){
  digitalWrite(5, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(5, LOW);
  // Serial2.println("HEllo");
  //digitalWrite(5, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);   
}

void NMI_Handler() {
  EIC->NMIFLAG.bit.NMI = 1;                         // Clear the NMI interrupt flag
  // PORT->Group[g_APinDescription[LED_BUILTIN].ulPort].OUTTGL.reg = 
  //   1 << g_APinDescription[LED_BUILTIN].ulPin;      // Toggle the LED_BUILTIN
  // Serial.println("Pressed enter");
  enter_pressed = true;
  delayMicroseconds(1000);
}
// void enter(){

//   return;
// }

void up(){
  // Serial.println("Pressed up");
  up_pressed = true;
  delayMicroseconds(1000);
  return;
}

void down_isr(){
  // Serial.println("Pressed down");
  down_pressed = true;
  delayMicroseconds(1000);
  return;
}

void left(){
  // Serial.println("Pressed left");
  delayMicroseconds(1000);
  return;
}

void right(){
  // Serial.println("Pressed right");
  delayMicroseconds(1000);
  return;
}

void printFileTree() {
  display.clearDisplay();
  display.setCursor(0,0);
  // display.setTextSize(0.5);
  display.println("Binaries: ");
  
  for(int i = current_display; i<current_display+3; i++) {
    // Serial.print("Here: ");
    // Serial.println(i);
    if(!start[i].folder && i != current_display){
      display.print("-> ");
      display.println(start[i].name);
    }
    else{
      display.print("[ ");
      display.print(start[i].name);
      display.println(" ]");
    }
    
  }

  display.display();
}

int printDirectory(File dir, int i) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    entry.getName(start[i].name, 15);
    start[i].loc = entry;
    Serial.println(start[i].name);
    if (entry.isDirectory()) {
      start[i].folder = true;
      i++;
      i = printDirectory(entry, i);
    } 
    else {
      i++;
      start[i].folder = false;
      // files have sizes, directories do not
    }

    entry.close();
  }
  return i;
}
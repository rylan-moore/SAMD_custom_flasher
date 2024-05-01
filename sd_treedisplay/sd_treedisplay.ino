// /*
//   Listfiles

//   This example shows how print out the files in a
//   directory on a SD card

//   The circuit:
//    SD card attached to SPI bus as follows:
//  ** MOSI - pin 11
//  ** MISO - pin 12
//  ** CLK - pin 13
//  ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

//   created   Nov 2010
//   by David A. Mellis
//   modified 9 Apr 2012
//   by Tom Igoe
//   modified 2 Feb 2014
//   by Scott Fitzgerald

//   This example code is in the public domain.

// */
// #include <SPI.h>
// #include <SD.h>

// #include <SPI.h>
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// #define SCREEN_WIDTH 128 // OLED display width, in pixels
// #define SCREEN_HEIGHT 32 // OLED display height, in pixels

// // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// // The pins for I2C are defined by the Wire-library. 
// // On an arduino UNO:       A4(SDA), A5(SCL)
// // On an arduino MEGA 2560: 20(SDA), 21(SCL)
// // On an arduino LEONARDO:   2(SDA),  3(SCL), ...
// #define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
// #define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// File root;

// void setup() {
//   // Open serial communications and wait for port to open:
//   Serial.begin(115200);
//   while (!Serial) {
//     ; // wait for serial port to connect. Needed for native USB port only
//   }

//   Serial.print("Initializing SD card...");

//   if (!SD.begin(23)) {
//     Serial.println("initialization failed!");
//     while (1);
//   }
//   Serial.println("initialization done.");

//   root = SD.open("/binaries/");

//     // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
//   if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
//     Serial.println(F("SSD1306 allocation failed"));
//     for(;;); // Don't proceed, loop forever
//   }
//   display.display();
//   delay(2000); // Pause for 2 seconds

//   // Clear the buffer
//   display.clearDisplay();

//   // Draw a single pixel in white
//   display.drawPixel(10, 10, SSD1306_WHITE);

//   // Show the display buffer on the screen. You MUST call display() after
//   // drawing commands to make them visible on screen!
//   display.display();
//   display.clearDisplay();
//   display.setCursor(0,0);
//   delay(2000);

//   printDirectory(root, 0);

//   Serial.println("done!");
// }

// void loop() {
//   // nothing happens after setup finishes.
// }

// void printDirectory(File dir, int numTabs) {
//   while (true) {

//     File entry =  dir.openNextFile();
//     if (! entry) {
//       // no more files
//       break;
//     }
//     for (uint8_t i = 0; i < numTabs; i++) {
//       Serial.print('\t');
//     }
//     Serial.print(entry.name());
//     // Serial.display();
//     if (entry.isDirectory()) {
//       Serial.println("/");
//       printDirectory(entry, numTabs + 1);
//     } else {
//       // files have sizes, directories do not
//       Serial.print("\t\t");
//       Serial.println(entry.size(), DEC);
//     }
//     entry.close();
//     // display.display();
//   }
// }
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SD.h>
#include <SPI.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int buttonPinUp = 2; // Change these pin numbers based on your setup
const int buttonPinDown = 3;
const int buttonPinSelect = 4;

File root;
File currentFile;

struct file_entry{
  String name;
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

const byte UP = 14;
const byte RIGHT = 9;

const byte LEFT = 10;
const byte DOWN = 11;
const byte ENTER = 8;

void setup() {
  Serial.begin(115200);
  while(!Serial){}
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

    pinMode(ENTER, INPUT_PULLDOWN);
    pinMode(RIGHT, INPUT_PULLDOWN);
    pinMode(LEFT, INPUT_PULLDOWN);
    pinMode(UP, INPUT_PULLDOWN);
    pinMode(DOWN, INPUT_PULLDOWN);

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

  // Initialize SD card
  if (!SD.begin(23)) {
    Serial.println("SD initialization failed!");
    return;
  }

  root = SD.open("/binaries");
  if(!root) {
    Serial.println("Failed to open SD root directory!");
    return;
  }
  printDirectory(root, 0);
  for(int j = 0; j<DSIZE; j++){
    if(start[j].name != ""){
      Serial.print(j);
      Serial.println(start[j].name);
    }
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("SD Card Files:");
  display.display();
  current_display = 0;
  printFileTree();
  currentFile = root;
  delay(2000);
}

void loop() {
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

  if (enter_pressed == true) {
    enter_pressed = false;
  }
  delay(10);
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
    start[i].name = entry.name();
    Serial.println(entry.name());
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

void NMI_Handler() {
  EIC->NMIFLAG.bit.NMI = 1;                         // Clear the NMI interrupt flag
  // PORT->Group[g_APinDescription[LED_BUILTIN].ulPort].OUTTGL.reg = 
  //   1 << g_APinDescription[LED_BUILTIN].ulPin;      // Toggle the LED_BUILTIN
  Serial.println("Pressed enter");
  enter_pressed = true;
  delayMicroseconds(1000);
}
// void enter(){

//   return;
// }

void up(){
  // Serial.println("Pressed up");
  delayMicroseconds(1000);
  up_pressed = true;
  return;
}

void down_isr(){
  // Serial.println("Pressed down");
  delayMicroseconds(1000);
  down_pressed = true;
  return;
}

void left(){
  // Serial.println("Pressed left");
  delayMicroseconds(1000);
  left_pressed = true;
  return;
}

void right(){
  // Serial.println("Pressed right");
  delayMicroseconds(1000);
  right_pressed = true;
  return;
}


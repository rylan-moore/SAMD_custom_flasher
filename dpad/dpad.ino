/**
 * @file dpad.ino
 * @author Rylan Moore (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-12-12
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "Arduino.h"

//look in here for pin issues: C:\Users\rylan\AppData\Local\Arduino15\packages\MattairTech_Arduino\hardware\samd\1.6.18-beta-b1\variants\MT_D21E_revB\variant.cpp

const byte UP = 14;
const byte RIGHT = 9;

const byte LEFT = 10;
const byte DOWN = 11;
const byte ENTER = 8;

void right();
void left();
void up();
void down_isr();

// the setup function runs once when you press reset or power the board
void setup() {
  delay(500);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(ENTER, INPUT_PULLDOWN);
  pinMode(RIGHT, INPUT_PULLDOWN);
  pinMode(LEFT, INPUT_PULLDOWN);
  pinMode(UP, INPUT_PULLDOWN);
  pinMode(DOWN, INPUT_PULLDOWN);

  pinMode(5, OUTPUT);
  // pinMode(6, OUTPUT); //XBEE Mosfet 
  Serial.begin(115200);
  // EIC->INTFLAG.bit.EXTINT10 = 1;
  // EIC->STATUS
  // NVIC->
  // attachInterrupt(digitalPinToInterrupt(ENTER), enter, RISING);
  // PORT->Group[g_APinDescription[LED_BUILTIN].ulPort].DIRSET.reg = 
  //   1 << g_APinDescription[LED_BUILTIN].ulPin;      // Set digital pin LED_BUILTIN to an OUTPUT

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

  //attach the normal interrupts -> Note that pins 10/11 needed changes in variant.cpp in order to be attached correctly. 
  attachInterrupt(digitalPinToInterrupt(UP), up, RISING);
  attachInterrupt(digitalPinToInterrupt(DOWN), down_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(RIGHT), right, RISING);
  attachInterrupt(digitalPinToInterrupt(LEFT), left, RISING);
  EIC->CONFIG[1].reg |= EIC_CONFIG_SENSE2_RISE;      
  // EIC->INTENSET.reg = EIC_INTENSET_EXTINT10;
  // EIC->INTENSET.reg = EIC_INTENSET_EXTINT11;
  // Serial2.begin(9600);
  // digitalWrite(6, HIGH);
}

// the loop function runs over and over again forever
void loop() {
  // Serial.println("Hello");
  
  digitalWrite(5, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(5, LOW);
  // Serial2.println("HEllo");
  //digitalWrite(5, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
  // Serial.print("Right: ");
  // Serial.println(digitalRead(RIGHT));

  // Serial.print("Left: ");
  // Serial.println(digitalRead(LEFT));

  // Serial.print("UP: ");
  // Serial.println(digitalRead(UP));

  // Serial.print("DOWN: ");
  // Serial.println(digitalRead(DOWN));

  // Serial.print("Enter: ");
  // Serial.println(digitalRead(ENTER));
}

void NMI_Handler() {
  EIC->NMIFLAG.bit.NMI = 1;                         // Clear the NMI interrupt flag
  // PORT->Group[g_APinDescription[LED_BUILTIN].ulPort].OUTTGL.reg = 
  //   1 << g_APinDescription[LED_BUILTIN].ulPin;      // Toggle the LED_BUILTIN
  Serial.println("Pressed enter");
  delayMicroseconds(1000);
}
// void enter(){

//   return;
// }

void up(){
  Serial.println("Pressed up");
  delayMicroseconds(1000);
  return;
}

void down_isr(){
  Serial.println("Pressed down");
  delayMicroseconds(1000);
  return;
}

void left(){
  Serial.println("Pressed left");
  delayMicroseconds(1000);
  return;
}

void right(){
  Serial.println("Pressed right");
  delayMicroseconds(1000);
  return;
}
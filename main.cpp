//WHOLE HOUSE VENTILATION SYSTEM
//Jennifer Herrera Sandoval, Anthony Rios-Jauregui, & Matt Chaiyasit
//ECET 40900
//4/28/2023

#include <Arduino.h>
#include "Adafruit_SHT31.h"
#include "Adafruit_SGP30.h"
#include <Wire.h> //I2C library
#include <SPI.h>
#include "BluetoothSerial.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <Fonts/FreeSans9pt7b.h>

//OLED GPIO pins
#define OLED_MOSI 23
#define OLED_CLK 18
#define OLED_DC 17
#define OLED_CS 5
#define OLED_RESET 16

// Color definitions
#define BLACK   0x0000
#define WHITE	0XFFFF
#define BLUE	0x2AF9
#define PURPLE	0xA29A
#define RED   	0xCA08
#define CYAN  	0x5E17
#define GREEN  	0x6E0B
#define YELLOW 	0xEEE7

//Create OLED object
Adafruit_SSD1351 display = Adafruit_SSD1351(128, 128, OLED_CS, OLED_DC, OLED_MOSI, OLED_CLK, OLED_RESET); 

//Variables for OLED
char intro[] = "  House Ventilation System  ";
int cursor = 0;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
//Create bluetooth object
BluetoothSerial SerialBT;

//Create tvoc/co2 object
Adafruit_SGP30 sgp;

//Create temp & humidity sensor object
Adafruit_SHT31 sht31 = Adafruit_SHT31();

//Variables for air sensors
float temp;
float hum;
int tvoc;
int co2;
int counter;

//Variables for motion sensor
const int motionSensor = 19;
int currentState = LOW;
int previousState = LOW;

//GPIO for fan
const int fanPWM = 12;
const int tachPin = 27;
int rpmCount;
int startTime;
int rpm;

//PWM settings
const int freq = 4000; //PWM requires 4kHz
const int channel = 0;
const int resolution = 8; //0-255

//GPIO fan control
const int modePin = 4;
const int potPin = 36;
const int lowLed = 32;
const int midLed= 33;
const int highLed =26;

//Bitmap of VOC icon
const unsigned char bitmap_voc [] PROGMEM = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 
	0xf8, 0x00, 0x3f, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0x80, 0x00, 0x03, 0xff, 0xff, 0xfc, 0xff, 
	0xff, 0xfe, 0x00, 0x00, 0x00, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x3f, 0xff, 
	0xfc, 0xff, 0xff, 0xe0, 0x00, 0x0f, 0xc0, 0x0f, 0xff, 0xfc, 0xff, 0xff, 0x80, 0x00, 0x0f, 0xf8, 
	0x07, 0xff, 0xfc, 0xff, 0xff, 0x00, 0x00, 0x0f, 0xfe, 0x03, 0xff, 0xfc, 0xff, 0xfe, 0x00, 0x00, 
	0x0f, 0xff, 0x81, 0xff, 0xfc, 0xff, 0xfc, 0x00, 0x00, 0x07, 0xff, 0xe0, 0x7f, 0xfc, 0xff, 0xf8, 
	0x00, 0x00, 0x07, 0xff, 0xf0, 0x7f, 0xfc, 0xff, 0xf0, 0x00, 0x00, 0x07, 0xff, 0xf8, 0x3f, 0xfc, 
	0xff, 0xe0, 0x00, 0x00, 0x03, 0xff, 0xfc, 0x1f, 0xfc, 0xff, 0xc0, 0x00, 0x00, 0x03, 0xff, 0xfe, 
	0x0f, 0xfc, 0xff, 0xc0, 0x00, 0x00, 0x01, 0xff, 0xff, 0x07, 0xfc, 0xff, 0x80, 0x00, 0x00, 0x01, 
	0xff, 0xff, 0x87, 0xfc, 0xff, 0x80, 0x00, 0x00, 0x00, 0xff, 0xff, 0x83, 0xfc, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xc3, 0xfc, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xe1, 0xfc, 0xfe, 
	0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xe1, 0xfc, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xf0, 
	0xfc, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xf0, 0xfc, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x0f, 
	0xff, 0xf0, 0xfc, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xf8, 0x7c, 0xfc, 0x00, 0x00, 0x00, 
	0x00, 0x03, 0xff, 0xf8, 0x7c, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xf8, 0x7c, 0xf8, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x7f, 0xf8, 0x7c, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x7c, 
	0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xfc, 0x7c, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
	0xfc, 0x3c, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x3c, 0xf8, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x3c, 0xf8, 0x18, 0x07, 0x0f, 0xe0, 0x07, 0xf0, 0x00, 0x3c, 0xf8, 0x1c, 0x0f, 
	0x1f, 0xf0, 0x1f, 0xf8, 0x00, 0x3c, 0xf8, 0x1c, 0x0e, 0x3f, 0xfc, 0x3f, 0xfc, 0x00, 0x7c, 0xf8, 
	0x0e, 0x0e, 0x78, 0x1c, 0x78, 0x1e, 0x00, 0x7c, 0xf8, 0x0e, 0x0c, 0xf0, 0x1e, 0x70, 0x00, 0x00, 
	0x7c, 0xfc, 0x0e, 0x1c, 0xe0, 0x0e, 0x70, 0x00, 0x00, 0x7c, 0xfc, 0x07, 0x1c, 0xe0, 0x0e, 0x60, 
	0x00, 0x00, 0x7c, 0xfc, 0x07, 0x38, 0xe0, 0x0e, 0x60, 0x00, 0x00, 0xfc, 0xfc, 0x03, 0x38, 0xe0, 
	0x0e, 0x60, 0x00, 0x00, 0xfc, 0xfe, 0x03, 0xb0, 0xe0, 0x0e, 0x70, 0x00, 0x00, 0xfc, 0xfe, 0x03, 
	0xf0, 0xf0, 0x1e, 0x70, 0x00, 0x01, 0xfc, 0xfe, 0x01, 0xf0, 0x78, 0x3c, 0x38, 0x1e, 0x01, 0xfc, 
	0xff, 0x01, 0xe0, 0x3f, 0xf8, 0x3f, 0xfc, 0x03, 0xfc, 0xff, 0x01, 0xe0, 0x1f, 0xf0, 0x1f, 0xf8, 
	0x03, 0xfc, 0xff, 0x80, 0xc0, 0x07, 0xe0, 0x07, 0xe0, 0x07, 0xfc, 0xff, 0xc0, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x07, 0xfc, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xfc, 0xff, 0xe0, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0xff, 
	0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 
	0xfc, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfc, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x01, 0xff, 0xfc, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x07, 0xff, 0xfc, 0xff, 0xff, 0xe0, 0x00, 
	0x00, 0x00, 0x0f, 0xff, 0xfc, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfc, 0xff, 0xff, 
	0xfc, 0x00, 0x00, 0x00, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0x00, 0x00, 0x03, 0xff, 0xff, 0xfc, 
	0xff, 0xff, 0xff, 0xf0, 0x00, 0x1f, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xef, 0xff, 0xff, 
	0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xfc
};
//Bitmap for fan icon
const unsigned char bitmap_fan [] PROGMEM = {
	0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 
	0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xfc, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x0f, 
	0xff, 0xff, 0xff, 0xff, 0xe0, 0x01, 0xff, 0xff, 0x80, 0x07, 0xff, 0xff, 0xff, 0xff, 0x80, 0x0f, 
	0xff, 0xff, 0xe0, 0x01, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x3f, 0xff, 0xff, 0xfc, 0x00, 0xff, 0xff, 
	0xff, 0xfc, 0x01, 0xff, 0xfe, 0x00, 0xff, 0x80, 0x3f, 0xff, 0xff, 0xf8, 0x03, 0xff, 0xfe, 0x00, 
	0x1f, 0xc0, 0x1f, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xfe, 0x00, 0x07, 0xf0, 0x0f, 0xff, 0xff, 0xe0, 
	0x1f, 0xff, 0xff, 0xfc, 0x00, 0xf8, 0x07, 0xff, 0xff, 0xc0, 0x3f, 0xff, 0xff, 0xff, 0xc0, 0x3c, 
	0x03, 0xff, 0xff, 0x80, 0xfc, 0x03, 0xff, 0x07, 0xf8, 0x1f, 0x01, 0xff, 0xff, 0x01, 0xe0, 0x00, 
	0x7f, 0x00, 0x7e, 0x07, 0x80, 0xff, 0xfe, 0x03, 0xc0, 0x00, 0x1f, 0x00, 0x1f, 0x03, 0xc0, 0x7f, 
	0xfc, 0x07, 0x00, 0x00, 0x07, 0xe0, 0x03, 0xc1, 0xe0, 0x7f, 0xfc, 0x07, 0x00, 0x00, 0x03, 0xfe, 
	0x00, 0xe1, 0xe0, 0x3f, 0xf8, 0x0e, 0x00, 0x00, 0x01, 0xff, 0xe0, 0x7f, 0xf8, 0x1f, 0xf8, 0x1c, 
	0x00, 0x00, 0x00, 0xff, 0xf8, 0x1f, 0xf8, 0x1f, 0xf0, 0x3c, 0x00, 0x00, 0x00, 0xff, 0xfc, 0x1f, 
	0xfc, 0x0f, 0xe0, 0x38, 0x00, 0x00, 0x00, 0x7f, 0xff, 0x3f, 0xfc, 0x0f, 0xe0, 0x78, 0x00, 0x00, 
	0x00, 0x3f, 0xff, 0xff, 0xfe, 0x07, 0xe0, 0xf8, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0x07, 
	0xc0, 0xf8, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xf8, 0x0f, 0x03, 0xc0, 0xf8, 0x00, 0x00, 0x00, 0x1f, 
	0xff, 0xf0, 0x07, 0x03, 0x81, 0xf8, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xc0, 0x03, 0x81, 0x81, 0xf8, 
	0x00, 0x00, 0x00, 0x0f, 0xff, 0x80, 0x01, 0x81, 0x83, 0xfc, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x00, 
	0x00, 0xc1, 0x03, 0xfe, 0x00, 0x00, 0x1f, 0xff, 0xfe, 0x00, 0x00, 0xc0, 0x07, 0xff, 0x00, 0x00, 
	0x20, 0x07, 0xf8, 0x00, 0x00, 0x60, 0x07, 0xff, 0x80, 0x00, 0xc0, 0x03, 0xf0, 0x00, 0x00, 0x60, 
	0x07, 0xff, 0xf0, 0x01, 0x80, 0x01, 0xe0, 0x00, 0x00, 0x20, 0x07, 0xff, 0xfe, 0x01, 0x00, 0x00, 
	0xc0, 0x00, 0x00, 0x20, 0x07, 0xff, 0xff, 0xc2, 0x00, 0x00, 0x40, 0x00, 0x00, 0x20, 0x07, 0xff, 
	0xff, 0xfc, 0x00, 0x00, 0x20, 0x00, 0x00, 0x20, 0x07, 0x8c, 0xff, 0xfc, 0x00, 0x00, 0x20, 0x00, 
	0x00, 0x20, 0x07, 0x0c, 0x7f, 0xfc, 0x03, 0xc0, 0x20, 0x00, 0x00, 0x60, 0x07, 0x0c, 0x7f, 0xfc, 
	0x03, 0xe0, 0x20, 0x00, 0x00, 0x70, 0x0f, 0x0c, 0x7f, 0xfc, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x70, 
	0x0f, 0x0c, 0x7f, 0xfc, 0x07, 0xe0, 0x00, 0x00, 0x00, 0xf0, 0x07, 0x0c, 0x7f, 0xfc, 0x07, 0xe0, 
	0x20, 0x00, 0x00, 0xf0, 0x07, 0x8c, 0x3f, 0xfc, 0x01, 0xc0, 0x20, 0x00, 0x01, 0xe0, 0x07, 0x8c, 
	0x3f, 0xfc, 0x00, 0x00, 0x20, 0x00, 0x01, 0xe0, 0x07, 0x84, 0x3f, 0xfe, 0x00, 0x00, 0x20, 0x00, 
	0x03, 0xe0, 0x07, 0x86, 0x3f, 0xe2, 0x00, 0x00, 0x40, 0x00, 0x07, 0xe0, 0x07, 0x86, 0x1f, 0xe1, 
	0x00, 0x00, 0x80, 0x00, 0x0f, 0xe0, 0x07, 0xc7, 0x1f, 0xc1, 0x80, 0x01, 0x80, 0x00, 0x3f, 0xe0, 
	0x07, 0xc7, 0x1f, 0xc0, 0xc0, 0x03, 0x00, 0x00, 0xff, 0xe0, 0x07, 0xc7, 0x0f, 0x80, 0x30, 0x0f, 
	0x00, 0x03, 0xff, 0xe0, 0x03, 0xe3, 0x0f, 0x80, 0x1f, 0xff, 0xf8, 0x3f, 0xff, 0xc0, 0x83, 0xe1, 
	0x87, 0x80, 0x00, 0x1f, 0xff, 0xfc, 0xff, 0xc1, 0x81, 0xe1, 0x87, 0x00, 0x00, 0x0f, 0xff, 0xfc, 
	0x7f, 0x81, 0x81, 0xf0, 0xc3, 0x00, 0x00, 0x0f, 0xff, 0xf8, 0x63, 0x81, 0xc0, 0xf0, 0xe3, 0x00, 
	0x00, 0x07, 0xff, 0xf0, 0xe3, 0x03, 0xc0, 0xf8, 0x67, 0x00, 0x00, 0x07, 0xff, 0xe0, 0xc3, 0x03, 
	0xe0, 0xf8, 0x7f, 0x00, 0x00, 0x07, 0xff, 0xc1, 0x87, 0x07, 0xe0, 0x7c, 0x3f, 0x00, 0x00, 0x07, 
	0xff, 0x83, 0x86, 0x07, 0xf0, 0x3e, 0x3f, 0x00, 0x00, 0x03, 0xff, 0x07, 0x0c, 0x07, 0xf0, 0x3f, 
	0x7f, 0x00, 0x00, 0x03, 0xfe, 0x0e, 0x1c, 0x0f, 0xf8, 0x1f, 0xff, 0x00, 0x00, 0x01, 0xfc, 0x1c, 
	0x18, 0x1f, 0xf8, 0x1f, 0xff, 0x00, 0x00, 0x01, 0xf0, 0x38, 0x38, 0x1f, 0xfc, 0x07, 0xff, 0x00, 
	0x00, 0x01, 0xc0, 0x70, 0x60, 0x3f, 0xfe, 0x07, 0xff, 0x80, 0x00, 0x01, 0x81, 0xe0, 0xe0, 0x3f, 
	0xfe, 0x03, 0xff, 0x80, 0x00, 0x00, 0x83, 0x81, 0xc0, 0x7f, 0xff, 0x01, 0xff, 0xc0, 0x00, 0x00, 
	0xcf, 0x07, 0x80, 0xff, 0xff, 0x80, 0xff, 0xe0, 0x00, 0x00, 0xfc, 0x0f, 0x01, 0xff, 0xff, 0xc0, 
	0x3f, 0xe0, 0x00, 0x01, 0xe0, 0x1c, 0x03, 0xff, 0xff, 0xe0, 0x1f, 0xf0, 0x00, 0x01, 0xe0, 0x78, 
	0x07, 0xff, 0xff, 0xf0, 0x0f, 0xf8, 0x00, 0x03, 0xe1, 0xf0, 0x0f, 0xff, 0xff, 0xf8, 0x03, 0xfc, 
	0x00, 0x07, 0xe7, 0xc0, 0x1f, 0xff, 0xff, 0xfc, 0x01, 0xfe, 0x00, 0x07, 0xff, 0x80, 0x3f, 0xff, 
	0xff, 0xff, 0x00, 0x3f, 0x80, 0x3f, 0xfc, 0x00, 0x7f, 0xff, 0xff, 0xff, 0x80, 0x0f, 0xf9, 0xff, 
	0xf0, 0x01, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x01, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xff, 
	0xf0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x3f, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 
	0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff
};
//Intro message for oled
void introMessage()
{
  display.setTextColor(WHITE, BLUE);
  int textLen = strlen(intro); //calculate length
  if (cursor == (textLen-1) ) //reset value to roll over
  {
    cursor = 0;
  }
  display.setCursor(0 , 16);
  if (cursor < textLen - 34)  //this loop is for 30 character
  {
    for (int charr = cursor; charr < cursor + 34; charr++)
    {
      display.print(intro[charr]);  //print message
    }
  }
  else
  {
    for (int charr = cursor; charr < (textLen - 1); charr++)
    {//prints character of current string
      display.print(intro[charr]);  //print message 
      
    }
    for (int charr = 0; charr <= 34 - (textLen - cursor); charr++)
    {//prints remaining characters
      display.print(intro[charr]);//print message
    }
  }
  cursor++;  
}
//Helps get RPM from tach
void IRAM_ATTR rpmReading()
{
  rpmCount++;
}
void setup() 
{
  //Establish serial communication
  Serial.begin(115200);

  //Establish bluetooth serial object
  SerialBT.begin("ESP32");

  //Establish I2C communication
  Wire.begin();

  //Initialize display
  display.begin();
  display.setRotation(3);
  display.fillScreen(BLUE);
  display.setTextSize(4);
  display.setTextWrap(false);
  display.fillRect(25, 50, 80, 80, WHITE);
  display.drawBitmap(25, 50, bitmap_fan, 80, 80, BLUE); //draws fan
  for (int t=0; t<23; t++)
  {
		introMessage(); //generates intro scrolling text
    delay(100);
  }
  

  //Initialize temp & humidity sensor
  if (sht31.begin(0x44))
  {
    Serial.println("Temperature/humidity sensor found");
  }
  else
  {
    Serial.println("Temperature/humidity sensor NOT found");
  }
  
  //Initialize air quality sensor
  if (sgp.begin())
  {
    Serial.println("SPG found");
  }
  else 
  {
    Serial.println("SPG NOT found");
  }

  //Initialize for air quality readings
  sgp.IAQinit(); 

  //Set PIR motion sensor as an input
  pinMode(motionSensor, INPUT);

  //Set LED output
  pinMode(lowLed, OUTPUT);
  pinMode(midLed, OUTPUT);
  pinMode(highLed, OUTPUT);

  //Set toggle switch as input for fan control
  pinMode(modePin, INPUT_PULLUP);
  //Set pin as output
  pinMode(fanPWM, OUTPUT);
  //Configure PWM functionalities
  ledcSetup(channel, freq, resolution);
  //Attach the channel to the GPIO 
  ledcAttachPin(fanPWM, channel);
  //Set tachometer as input for RPM
  pinMode(tachPin, INPUT);
  //Attach interrupt to tachometer
  attachInterrupt(digitalPinToInterrupt(tachPin),rpmReading, RISING);
}
void sendDataBT()
{
  //Convert data to string so it can send it to app
  String hold_temp = String(temp);
  String hold_hum = String(hum);
  String hold_tvoc = String(tvoc);
  String hold_co2 = String(co2);
  String hold_rpm = String(rpm);

  //Send data using bluetooth
  SerialBT.print(hold_temp);
  SerialBT.print(",");
  SerialBT.print(hold_hum);
  SerialBT.print(",");
  SerialBT.print(hold_tvoc);
  SerialBT.print(",");
  SerialBT.print(hold_co2);
  SerialBT.print(",");
  SerialBT.print(hold_rpm);
  SerialBT.println(",");
}
void getSensorData()
{
  //Get temp reading and convert C to F
  temp = ((sht31.readTemperature() * 1.8) + 32);
  //Get humidtiy reading
  hum = sht31.readHumidity();
  //Check if getting good temp reading and print
  if (! isnan(temp)) 
  {  // check if 'is not a number'
    Serial.print("Temp *F = "); 
    Serial.println(temp);
  } 
  else 
  { 
    Serial.println("Failed to read temperature");
  }
  //Check if getting good hum reading and print
  if (! isnan(hum)) 
  {  // check if 'is not a number'
    Serial.print("Hum % = "); 
    Serial.println(hum);
  } 
  else 
  { 
    Serial.println("Failed to read humidity");
  }
  delay(1000);
  //Measure CO2 AND TVOC levels
  if (! sgp.IAQmeasure())
  {
    Serial.println("Measurement failed");
  }
  //Get TVOC and CO2 readings and print them
  tvoc = sgp.TVOC;
  co2 = sgp.eCO2;
  Serial.print("CO2: ");
  Serial.print(co2);
  Serial.println(" ppm");
  Serial.print("TVOC: ");
  Serial.print(tvoc);
  Serial.println(" ppb");
  delay(1000);
  
  //Maintain Baseline readings for CO2 and TVOC
  counter++;
  if (counter == 30)
  {
    counter=0;
  }
  uint16_t TVOC_base, eCO2_base;
    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
      Serial.println("Failed to get baseline readings");
      return;
    }
}
void setScreenSaver()
{
  display.fillScreen(BLUE);
	display.setTextSize(3);
	display.setTextColor(WHITE, BLUE);
	display.fillRect(30, 2, 70, 70, WHITE);
	display.drawBitmap(30, 2, bitmap_voc, 70, 70, BLUE);
	display.drawRoundRect(20, 70, 90, 58, 5, WHITE);
	display.setCursor(40, 75);
	display.print(tvoc);
	display.setCursor(40, 100);
	display.print("ppb");
}
void updateOled()
{
  //Clear screen
  display.fillScreen(BLACK);
  //Display temperature
  display.setTextSize(1);
  display.setCursor(0,0);
  display.setFont();
  display.setTextColor(PURPLE);
  display.print("Temperature: ");
  display.setCursor(7, 15);
	display.fillCircle(2, 15, 2, PURPLE);
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(WHITE, BLACK);
  display.print(temp);
  display.print(" *F");
  //Display humidity
  display.setCursor(0, 31);
  display.setFont();
  display.setTextColor(RED);
  display.print("Humidity: ");
  display.setCursor(7, 41);
	display.fillCircle(2, 41, 2, RED);
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(WHITE, BLACK);
  display.print(hum);
  display.print(" % RH"); 
  //Display CO2
  display.setCursor(0, 57);
  display.setFont();
  display.setTextColor(GREEN);
  display.print("CO2: ");
  display.setCursor(7, 67);
	display.fillCircle(2, 67, 2, GREEN);
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(WHITE, BLACK);
  display.print(co2);
  display.print(" ppm"); 
  //Display TVOC
  display.setCursor(0, 83);
  display.setFont();
  display.setTextColor(CYAN);
  display.print("TVOC: ");
  display.setCursor(7, 93);
	display.fillCircle(2, 93, 2, CYAN);
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(WHITE, BLACK);
  display.print(tvoc);
  display.print(" ppb"); 
  //Display RPM
  display.setCursor(0, 110);
  display.setFont();
  display.setTextColor(BLUE);
  display.print("Fan Speed: ");
  display.setCursor(7, 120);
	display.fillCircle(2,120, 2, BLUE);
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(WHITE, BLACK);
  display.print(rpm);
  display.print(" RPM");
  //Dsiplay fan control mode
	display.drawCircle(105, 80, 22, YELLOW);
	display.setCursor(95, 75);
	display.setFont();
	display.setTextColor(YELLOW);
	display.print("Mode");
	display.setTextColor(WHITE, BLACK);
  int mode2 = digitalRead(modePin);
	if(mode2)
	{
    display.setCursor(95, 84);
  	display.print("AUTO");
	}
	else
	{
    display.setCursor(88, 84);
		display.print("MANUAL");
	}
}
void automaticControl()
{
  if ((hum <= 9) || (hum >= 91) || (co2 >= 1101) || (tvoc >= 301))
  {
    ledcWrite(channel, 255);//99% speed
    Serial.println("Speed 3");
    digitalWrite(highLed, HIGH);
    digitalWrite(midLed, LOW);
    digitalWrite(lowLed, LOW);
   
    
  }
  else if ((hum <= 19 && hum >= 10) || (hum <= 90 && hum >= 76) || (co2 <= 1110 && co2 >= 1051) || (tvoc <= 300 && tvoc >= 251))
  {
    ledcWrite(channel, 120);//66% speed
    Serial.println("Speed 2");
    digitalWrite(highLed, LOW);
    digitalWrite(midLed, HIGH);
    digitalWrite(lowLed, LOW);
    
  }
  else if ((hum <= 30 && hum >= 20) || (hum <= 75 && hum >= 50) || (co2 <= 1050 && co2 >= 1025) || (tvoc <= 250 && tvoc >= 225))
  {
    ledcWrite(channel, 85);//33% speed
    Serial.println("Speed 1");
    digitalWrite(highLed, LOW);
    digitalWrite(midLed, LOW);
    digitalWrite(lowLed, HIGH);
   
  }
  else
  {
    ledcWrite(channel, 0); //fan off
    digitalWrite(highLed, LOW);
    digitalWrite(midLed, LOW);
    digitalWrite(lowLed, LOW);
    Serial.println("Fan off");

  }
}
void manualControl()
{
  //Read potentiometer value
  int potVal = analogRead(potPin);
  //Scales value from potentiometer to a value betweeen 0-255
  int speed = map(potVal, 0, 4095, 0, 255);
  ledcWrite(channel, speed);
  Serial.println(speed);
  if (speed > 0 && speed <= 85)
  {
    digitalWrite(highLed, LOW);
    digitalWrite(midLed, LOW);
    digitalWrite(lowLed, HIGH);
  }
  else if (speed > 85 && speed <=120)
  {
    digitalWrite(highLed, LOW);
    digitalWrite(midLed, HIGH);
    digitalWrite(lowLed, LOW);
  }
  else if (speed > 120)
  {
    digitalWrite(highLed, HIGH);
    digitalWrite(midLed, LOW);
    digitalWrite(lowLed, LOW);
  }
  else
  {
    digitalWrite(highLed, LOW);
    digitalWrite(midLed, LOW);
    digitalWrite(lowLed, LOW);
  }
}
void getRPM()
{
  //Set as current time
  startTime = millis();
  //Reset rpm count
  rpmCount = 0;
  //Wait for one second to pass
  while ((millis() - startTime) < 1000)
  {
    
  }
  rpm = rpmCount * 60 / 2;
  Serial.print(rpm);
  Serial.println(" rpm");
}
void checkMovement()
{
  previousState = currentState; // store old state
  currentState = digitalRead(19);   // read new state

  if (previousState == LOW && currentState == HIGH) 
  {   // pin state change: LOW -> HIGH
    Serial.println("Motion detected!");
    // TODO: turn on alarm, light or activate a device ... here
    updateOled();
    //Keep that display on for 30 seconds
    delay(30000);
  }
  else if (previousState == HIGH && currentState == LOW) 
  {   // pin state change: HIGH -> LOW
    Serial.println("Motion stopped!");
    // TODO: turn off alarm, light or deactivate a device ... here
    setScreenSaver();
  }
}
void loop() 
{
  //Get sensor data
  getSensorData();
  //Check which mode user wants
  int mode = digitalRead(modePin);
  if (mode)
  {
    automaticControl();
    Serial.println("AUTO");
  }
  else
  {
    manualControl();
    Serial.println("MANUAL");
  }
  //Get fan RPM
  getRPM();
  //Check if person is near system
  checkMovement();
  //Send data via bluetooth
  sendDataBT();
  delay(1000);
}

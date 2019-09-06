/*
 Name:		LoadcellPLA.ino
 Created:	9/6/2019 4:02:46 PM
 Author:	Benjamin Odland Skare
*/


// Loadcell
#include <Q2HX711.h>
const byte hx711_data_pin = 5;
const byte hx711_clock_pin = 6;
Q2HX711 hx711(hx711_data_pin, hx711_clock_pin);


// Display
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void setup() {
	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
		Serial.println(F("SSD1306 allocation failed"));
		for (;;); // Don't proceed, loop forever
	}
}

// En rull med PLA-plast: 340 m, 1 kg
// 334 gram på trommelen

long value;
double m_pr_g = 0.34;

void loop() {

	value = (hx711.read() / 1000.0) - 8350;
	value = value * 1.07;
	value = value - 344;

	display.clearDisplay();
	display.setTextSize(2);
	display.setTextColor(WHITE);
	display.setCursor(0, 0);

	display.print(value);
	display.println(" g");

	int m = value * 0.34;

	display.print(m);
	display.print(" m");

	display.display();
	delay(100);
}

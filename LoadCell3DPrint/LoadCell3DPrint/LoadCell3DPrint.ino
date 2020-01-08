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
#include <EEPROM.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BTN_UP_PIN 2
#define BTN_DOWN_PIN 3

long value;
double m_pr_g = 0.34;
int16_t weight = 0x158;
int address = 0;
byte valueb;


void setup() {
	byte h = EEPROM.read(1);
	byte l = EEPROM.read(0);
	Serial.println(h, HEX);
	Serial.println(l, HEX);

	weight = (h << 8) + l;
	Serial.println(weight, HEX);


	pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
	pinMode(BTN_UP_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(BTN_UP_PIN), adjust_up, FALLING);
	attachInterrupt(digitalPinToInterrupt(BTN_DOWN_PIN), adjust_down, FALLING);

	


	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
		Serial.println(F("SSD1306 allocation failed"));
		for (;;); // Don't proceed, loop forever
	}
}

// En rull med PLA-plast: 340 m, 1 kg
// 334 gram på trommelen



void loop() {
	value = (hx711.read() / 1000.0) - 8350;
	value = value * 1.07;
	value = value - weight;

	display.clearDisplay();
	display.setTextSize(2);
	display.setTextColor(WHITE);
	display.setCursor(0, 0);

	display.print(value);
	display.println(" g");

	int m = value * 0.34;
	

	display.print(m);
	display.print(" m");

	display.setCursor(100, 20);
	display.setTextSize(1);

	display.print(weight);

	display.display();
	delay(100);
}


void adjust_up() {
	weight++;

	eeprom_write();
}

void adjust_down() {
	weight--;

	eeprom_write();
}

void eeprom_write() {
	uint8_t  bytes[2];

	bytes[0] = weight >> 8;     // high byte (0x12)
	bytes[1] = weight & 0x00FF; // low byte (0x34)

	EEPROM.write(0, bytes[1]);
	EEPROM.write(1, bytes[0]);

	/*Serial.print(bytes[0], HEX);
	Serial.print("   -   ");
	Serial.print(bytes[1], HEX);
	Serial.print("   -   ");
	Serial.println(weight, HEX);*/
}
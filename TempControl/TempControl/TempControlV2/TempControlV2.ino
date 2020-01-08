/*
 Name:		TempControlV2.ino
 Created:	1/7/2020 11:47:47 PM
 Author:	47974
*/

// Display
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET     4 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Temperature sensor
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS  4
DeviceAddress TEMP_SENS;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Touch
#define BTN_DOWN_PIN 3
#define BTN_UP_PIN 2

// Setpoint temp
#include <EEPROM.h>
float setpoint = 10.0;
#define ADDRESS_H 0
#define ADDRESS_L 1



void setup() {
	byte h = EEPROM.read(ADDRESS_H);
	byte l = EEPROM.read(ADDRESS_L);
	Serial.println(h, HEX);
	Serial.println(l, HEX);
	setpoint = ((h << 8) + l) / 100.0;
	Serial.println(setpoint);

	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
		Serial.println(F("SSD1306 allocation failed"));
		for (;;); // Don't proceed, loop forever
	}

	sensors.begin();
	if (!sensors.getAddress(TEMP_SENS, 0)) Serial.println("Unable to find address for Device 0");
	sensors.setResolution(TEMP_SENS, 12);
	Serial.print("Device 0 Resolution: ");
	Serial.print(sensors.getResolution(TEMP_SENS), DEC);
	Serial.println();


	pinMode(BTN_DOWN_PIN, INPUT);
	pinMode(BTN_UP_PIN, INPUT);
	attachInterrupt(digitalPinToInterrupt(BTN_UP_PIN), adjust_up, RISING);
	attachInterrupt(digitalPinToInterrupt(BTN_DOWN_PIN), adjust_down, RISING);
}

// the loop function runs over and over again until power down or reset
void loop() {


	sensors.requestTemperatures();
	float temp = sensors.getTempC(TEMP_SENS);

	display.clearDisplay();
	display.setTextSize(2);
	display.setTextColor(WHITE);
	display.setCursor(0, 0);
	display.print("T: ");
	display.setTextSize(3);

	display.println(temp);
	display.setTextSize(2);

	display.print("SP: ");
	display.print(setpoint);
	display.display();
	delay(100);

	if (digitalRead(BTN_DOWN_PIN) && digitalRead(BTN_UP_PIN)) setpoint = 10.0;
}
void adjust_up() {
	setpoint += 0.5;
	eeprom_write();
}
void adjust_down() {
	setpoint -= 0.5;
	eeprom_write();
}
void eeprom_write() {
	uint8_t  bytes[2];
	int16_t data = setpoint * 100;
	bytes[0] = data >> 8;     // high byte (0x12)
	bytes[1] = data & 0x00FF; // low byte (0x34)

	EEPROM.write(ADDRESS_L, bytes[1]);
	EEPROM.write(ADDRESS_H, bytes[0]);

}
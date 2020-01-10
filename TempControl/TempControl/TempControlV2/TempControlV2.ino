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
double setpoint = 10.0;
#define ADDRESS_H 0
#define ADDRESS_L 1

// Cool, heat
double temp = 20;

bool state;
double outputVal;
#define COOL 10 
#define HEAT 11


#include <AutoPID.h>
#define OUTPUT_MIN -100
#define OUTPUT_MAX 100
#define KP 10
#define KI .005
#define KD .02
AutoPID myPID(&temp, &setpoint, &outputVal, OUTPUT_MIN, OUTPUT_MAX, KP, KI, KD);

unsigned long lastTempUpdate; //tracks clock time of last temp update
#define TEMP_READ_DELAY 800 //can only read digital temp sensor every ~750ms


bool updateTemperature() {
	if ((millis() - lastTempUpdate) > TEMP_READ_DELAY) {
		temp = sensors.getTempC(TEMP_SENS); //get temp reading
		//Serial.print(temp);
		//Serial.print("   ");

		//temp2 = temp;
		lastTempUpdate = millis();
		sensors.requestTemperatures(); //request reading for next time
		return true;
	}
	return false;
}//void updateTemperature


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

	pinMode(COOL, OUTPUT);
	pinMode(HEAT, OUTPUT);
	digitalWrite(COOL, LOW);
	digitalWrite(HEAT, LOW);

	pinMode(BTN_DOWN_PIN, INPUT);
	pinMode(BTN_UP_PIN, INPUT);
	attachInterrupt(digitalPinToInterrupt(BTN_UP_PIN), adjust_up, RISING);
	attachInterrupt(digitalPinToInterrupt(BTN_DOWN_PIN), adjust_down, RISING);


	while (!updateTemperature()) {} //wait until temp sensor updated
	myPID.setBangBang(3);
	myPID.setTimeStep(1000);
}

void loop() {
	updateTemperature();
	myPID.run();
	//Serial.println(outputVal);


	digitalWrite(HEAT, state);
	digitalWrite(COOL, myPID.atSetPoint(1));


	//sensors.requestTemperatures();
	//temp = sensors.getTempC(TEMP_SENS);

	display.clearDisplay();
	display.setTextSize(2);
	display.setTextColor(WHITE);
	display.setCursor(0, 0);
	display.print("T: ");
	display.setTextSize(3);

	display.println(temp);
	display.setTextSize(2);

	display.print("SP: ");
	display.println(setpoint);
	display.print(": ");
	display.print(outputVal);
	//Serial.println(setpoint);
	display.display();
	delay(1);

	if (digitalRead(BTN_DOWN_PIN) && digitalRead(BTN_UP_PIN)) setpoint = 10.0;
}
void adjust_up() {
	Serial.println("adjust up");
	setpoint += 0.5;
	myPID.reset();
	eeprom_write();
}
void adjust_down() {
	Serial.println("adjust down");

	setpoint -= 0.5;
	myPID.reset();

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
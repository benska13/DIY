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
DeviceAddress TEMP_SENS_BEER;
DeviceAddress TEMP_SENS_ROOM;
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
double temp_beer = 20;
double temp_room = 21;

bool state;
double outputVal;
#define COOL_OUT 10
#define FINE_OUT 11
#define HEAT_OUT 12
constexpr auto HEAT = 1;
constexpr auto COOL = 2;
constexpr auto REST = 0;
constexpr auto READY = 3;

unsigned long swichTime;
short mode = 0;

#include <AutoPID.h>
#define OUTPUT_MIN -200
#define OUTPUT_MAX 100
#define KP 10
#define KI .005
#define KD .02
AutoPID myPID(&temp_beer, &setpoint, &outputVal, OUTPUT_MIN, OUTPUT_MAX, KP, KI, KD);

unsigned long lastTempUpdate; //tracks clock time of last temp update
#define TEMP_READ_DELAY 800 //can only read digital temp sensor every ~750ms


unsigned long time = 0;

bool updateTemperature() {
	if ((millis() - lastTempUpdate) > TEMP_READ_DELAY) {
		temp_beer = sensors.getTempC(TEMP_SENS_BEER); //get temp reading
		temp_room = sensors.getTempC(TEMP_SENS_ROOM);
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
	//Serial.println(h, HEX);
	//Serial.println(l, HEX);
	setpoint = ((h << 8) + l) / 100.0;
	//Serial.println(setpoint);

	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
		Serial.println(F("SSD1306 allocation failed"));
		for (;;); // Don't proceed, loop forever
	}

	sensors.begin();
	if (!sensors.getAddress(TEMP_SENS_BEER, 0)) Serial.println("Unable to find address for Device 0");
	if (!sensors.getAddress(TEMP_SENS_ROOM, 1)) Serial.println("Unable to find address for Device 1");

	sensors.setResolution(TEMP_SENS_BEER, 12);
	sensors.setResolution(TEMP_SENS_ROOM, 12);

	//Serial.print("Device 0 Resolution: ");
	//Serial.print(sensors.getResolution(TEMP_SENS), DEC);
	//Serial.println();

	pinMode(COOL_OUT, OUTPUT);
	pinMode(HEAT_OUT, OUTPUT);
	pinMode(FINE_OUT, OUTPUT);
	digitalWrite(COOL_OUT, HIGH);
	digitalWrite(HEAT_OUT, HIGH);
	digitalWrite(FINE_OUT, LOW);

	pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
	pinMode(BTN_UP_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(BTN_UP_PIN), adjust_up, FALLING);
	attachInterrupt(digitalPinToInterrupt(BTN_DOWN_PIN), adjust_down, FALLING);


	while (!updateTemperature()) {} //wait until temp sensor updated
	myPID.setBangBang(4);
	myPID.setTimeStep(4000);

	swichTime = 0;
}

void loop() {
	updateTemperature();
	myPID.run();
	digitalWrite(FINE_OUT, !myPID.atSetPoint(0.5));

	if (millis()/1000 > time)		time++;


	if (time > swichTime)
	{
		switch (mode)
		{
		case READY:
			if (outputVal > 2) //Heat
			{
				time = 0;
				swichTime = outputVal;
				mode = HEAT;
			}
			if (outputVal < -5) // Cool
			{
				time = 0;
				swichTime = abs(outputVal)+50;
				mode = COOL;
			}
			break;
		case REST:
			mode = READY;
			break;
		case HEAT:
			mode = REST;
			time = 0;
			swichTime = 100 - outputVal;
			break;
		case COOL:
			mode = REST;
			time = 0;
			swichTime = 200 - abs(outputVal);
			break;
		default:
			mode = REST;
			break;
		}
	}


	delay(50);


	if (mode == COOL && outputVal < 0 && outputVal > 3)
	{
		mode = REST;
		//swichTime = millis() + (30 * 1000);

	}
	if (mode == HEAT && outputVal > 0 && outputVal < 3) 
	{
		mode = REST;
		//swichTime = millis() + (30 * 1000);

	}

	tempControl(mode);

	display.clearDisplay();
	display.setTextColor(WHITE);

	display.setTextSize(3);
	display.setCursor(0, 0);
	display.print("C:");
	display.println(temp_beer);

	display.setTextSize(2);
	display.print("SP: ");
	display.println(setpoint);

	display.setTextSize(1);
	display.print("P:");
	display.print(outputVal);

	
	display.print(" T:");
	display.println(time);


	display.print("Tsw.: ");
	display.println(swichTime);


	display.print("RT:");
	display.print(temp_room);
	display.print("   m:");
	display.println(mode);

	display.display();



	if (!digitalRead(BTN_DOWN_PIN) && !digitalRead(BTN_UP_PIN)) setpoint = 10.0;
}
void adjust_up() {
	delay(40);
	if (digitalRead(BTN_UP_PIN))
	{
		return;
	}
	setpoint += 0.5;
	myPID.reset();
	eeprom_write();
	time = 0;
	swichTime = 20;
}
void adjust_down() {
	delay(40);
	if (digitalRead(BTN_DOWN_PIN))
	{
		return;
	}
	setpoint -= 0.5;
	myPID.reset();
	eeprom_write();
	time = 0;
	swichTime = 20;
}
void eeprom_write() {
	uint8_t  bytes[2];
	int16_t data = setpoint * 100;
	bytes[0] = data >> 8;     // high byte (0x12)
	bytes[1] = data & 0x00FF; // low byte (0x34)

	EEPROM.write(ADDRESS_L, bytes[1]);
	EEPROM.write(ADDRESS_H, bytes[0]);

}
void tempControl(int mode) {
	if (mode == HEAT)
	{
		digitalWrite(HEAT_OUT, LOW);
		digitalWrite(COOL_OUT, HIGH);
	}
	else if (mode == COOL)
	{
		digitalWrite(HEAT_OUT, HIGH);
		digitalWrite(COOL_OUT, LOW);
	}
	else
	{
		digitalWrite(HEAT_OUT, HIGH);
		digitalWrite(COOL_OUT, HIGH);

	}

}

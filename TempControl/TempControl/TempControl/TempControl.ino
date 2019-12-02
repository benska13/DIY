/*
 Name:		TempControl.ino
 Created:	10/9/2019 8:26:03 PM
 Author:	Benjamin Odland Skare
*/

#include <OneWire.h>
#include <DallasTemperature.h>
constexpr auto ONE_WIRE_BUS = 2;
constexpr auto TEMP_AIR = 0;
constexpr auto TEMP_BARREL = 1;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


#include <LiquidCrystal.h>
#include <math.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 6);

constexpr auto RELAY_HEAT = 8;
constexpr auto RELAY_COOL = 9;
constexpr auto HEAT = 1;
constexpr auto COOL = 2;
constexpr auto REST = 0;
constexpr auto MARGIN = 1;
long time = 0;

void setup(void)
{
	sensors.begin();
	Serial.begin(9600);
	lcd.begin(16, 2);

	pinMode(RELAY_COOL, OUTPUT);
	digitalWrite(RELAY_COOL, HIGH);
	pinMode(RELAY_HEAT, OUTPUT);
	digitalWrite(RELAY_HEAT, HIGH);
	pinMode(LED_BUILTIN, OUTPUT);
}

void loop(void)
{
	digitalWrite(LED_BUILTIN, HIGH);
	delay(100);
	digitalWrite(LED_BUILTIN, LOW);

	//lcd.clear();
	lcd.flush();
	sensors.requestTemperatures();
	float tempAir = sensors.getTempCByIndex(TEMP_AIR);
	lcd.setCursor(0, 0);
	lcd.print("A:");
	lcd.print(tempAir);

	Serial.print(tempAir);
	Serial.print("    -    ");

	float tempBarrel = sensors.getTempCByIndex(TEMP_BARREL);
	lcd.setCursor(0, 1);
	lcd.print("B:");
	lcd.print(tempBarrel);
	Serial.print(tempBarrel);
	Serial.print("  -   ");
	int potValue = analogRead(A0);
	float tempTarget = map(potValue, 0, 1000, 50, 300) / 10.0;
	lcd.setCursor(11, 0);
	lcd.print(tempTarget);
	Serial.print(tempTarget);
	Serial.print("  -   ");
	lcd.setCursor(11, 1);
	lcd.print(time);
	Serial.println(time);

	int mode = REST;

	if (tempBarrel > tempTarget + 0.75)    // startCooling
	{
		if (time < 220)
		{
			tempControl(COOL);
			mode = COOL;
		}
		else tempControl(REST);

		time++;
	}
	else if (tempBarrel < tempTarget - 0.75)    // startHeating
	{
		if (time < 120)
		{
			tempControl(HEAT);
			mode = HEAT;
		}
		else tempControl(REST);

		time++;
	}
	/*
	if (mode == HEAT && tempBarrel > tempTarget + 0.2)
	{


	}
	if (mode == COOL && tempBarrel < tempTarget - 0.2)
	{
		tempControl(REST);

	}*/
	if (abs(tempBarrel - tempTarget) < 0.3)
	{

		tempControl(REST);
		time = 0;
	}

	if (digitalRead(RELAY_COOL) == HIGH && digitalRead(RELAY_HEAT) == HIGH)
	{
		tempControl(REST);
	}
	if (time > 350)
	{
		time = 0;
	}

	delay(800);
}


void tempControl(int mode) {
	if (mode == 1)
	{
		digitalWrite(RELAY_HEAT, LOW);
		digitalWrite(RELAY_COOL, HIGH);
	}
	else if (mode == 2)
	{
		digitalWrite(RELAY_HEAT, HIGH);
		digitalWrite(RELAY_COOL, LOW);
	}
	else
	{
		digitalWrite(RELAY_HEAT, HIGH);
		digitalWrite(RELAY_COOL, HIGH);

	}

}
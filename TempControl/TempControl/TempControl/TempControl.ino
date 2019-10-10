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
LiquidCrystal lcd(12, 11, 7, 6, 5, 4);

constexpr auto RELAY_HEAT = 8;
constexpr auto RELAY_COOL = 9;
constexpr auto HEAT = 1;
constexpr auto COOL = 2;
constexpr auto REST = 0;
constexpr auto MARGIN = 1;


void setup(void)
{
	sensors.begin();
	Serial.begin(9600);
	lcd.begin(16, 2);

	pinMode(RELAY_COOL, OUTPUT);
	digitalWrite(RELAY_COOL, HIGH);
	pinMode(RELAY_HEAT, OUTPUT);
	digitalWrite(RELAY_HEAT, HIGH);
}

void loop(void)
{
	sensors.requestTemperatures();
	float tempAir = sensors.getTempCByIndex(TEMP_AIR);
	lcd.setCursor(0, 0);
	lcd.print("A:");
	lcd.print(tempAir);
	float tempBarrel = sensors.getTempCByIndex(TEMP_BARREL);
	lcd.setCursor(0, 1);
	lcd.print("B:");
	lcd.print(tempBarrel);

	int potValue = analogRead(A0);
	float tempTarget = map(potValue, 0, 1000, 50, 300) / 10.0;
	lcd.setCursor(11, 0);
	lcd.print(tempTarget);


	if (tempBarrel > tempTarget + 1)
	{
		// startCooling
		tempControl(COOL);
	}
	else if (tempBarrel < tempTarget - 1)
	{
		// startHeating
		tempControl(HEAT);
	}
	else
	{
		tempControl(REST);
	}




	//if (tempBarrel > tempTarget + 1)
	//{
	//	if (tempAir > tempTarget -1)
	//	{
	//		// startCooling
	//		tempControl(COOL);
	//	}
	//	else
	//	{
	//		tempControl(REST);
	//	}
	//}
	//else if (tempBarrel < tempTarget - 1)
	//{
	//	if (tempAir < tempTarget + 1 )
	//	{
	//		// startHeating
	//		tempControl(HEAT);
	//	}
	//	else
	//	{
	//		tempControl(REST);
	//	}
	//}
	//else
	//{
	//	tempControl(REST);
	//}




	delay(100);
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
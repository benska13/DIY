/*
 Name:		TempControlV3.ino
 Created:	2/3/2020 9:02:13 PM
 Author:	Benjamin Skare
*/

//Screen
#include <Wire.h> 
#include <LiquidCrystal_I2C_DFR.h>
LiquidCrystal_I2C_DFR lcd(0);

// Temperature sensor
#include <OneWire.h>
#include <DallasTemperature.h>
constexpr auto ONE_WIRE_BUS = 6;
DeviceAddress TEMP_SENS_BEER;
DeviceAddress TEMP_SENS_ROOM;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
unsigned long lastTempUpdate; //tracks clock time of last temp update
constexpr auto TEMP_READ_DELAY = 800; //can only read digital temp sensor every ~750ms;


//Storage eeprom
#include <EEPROM.h>;
constexpr auto s_address = 0;
constexpr auto p_address = 2;
constexpr auto i_address = 4;
constexpr auto d_address = 6;

//Values

double s = 15;
double t = 20.0;
float tr = 25.0;

//Menu
int pos_menu = 0;
const int btn_menu = 12;
const int btn_up = 10;
const int btn_down = 11;
char menu[] = { 'p','i','d','s' };

// Heat/cool logic
constexpr auto COOL_OUT = 8;
constexpr auto FINE_OUT = 9;
constexpr auto HEAT_OUT = 7;
constexpr auto HEAT = 1;
constexpr auto COOL = 2;
constexpr auto REST = 0;
constexpr auto READY = 3;
unsigned long time = 0;
unsigned long swichTime;
short mode = 0;


// PID
double outputVal;
#include <AutoPID.h>
constexpr auto OUTPUT_MIN = -100;
constexpr auto OUTPUT_MAX = 100;


unsigned long co = 0;

void menu_control();
double read_eeprom(int address);
void write_eeprom(double valueFloat, int address);
bool updateTemperature();
void tempControl(int mode);
void print_menu();
double p = read_eeprom(p_address);
double i = read_eeprom(i_address);
double d = read_eeprom(d_address);
AutoPID myPID(&t, &s, &outputVal, OUTPUT_MIN, OUTPUT_MAX, p, i, d);

void setup() {
	lcd.begin(16, 2);
	lcd.backlight();
	lcd.clear();

	pinMode(btn_down, INPUT_PULLUP);
	pinMode(btn_menu, INPUT_PULLUP);
	pinMode(btn_up, INPUT_PULLUP);
	pinMode(HEAT_OUT, OUTPUT);
	pinMode(COOL_OUT, OUTPUT);
	pinMode(FINE_OUT, OUTPUT);
	digitalWrite(HEAT_OUT, HIGH);
	digitalWrite(COOL_OUT, HIGH);
	digitalWrite(FINE_OUT, HIGH);

	sensors.begin();
	if (!sensors.getAddress(TEMP_SENS_BEER, 1)) Serial.println("Unable to find address for Device 0");
	if (!sensors.getAddress(TEMP_SENS_ROOM, 0)) Serial.println("Unable to find address for Device 1");

	sensors.setResolution(TEMP_SENS_BEER, 12);
	sensors.setResolution(TEMP_SENS_ROOM, 12);

	while (!updateTemperature()) {} //wait until temp sensor updated



	/*p = read_eeprom(p_address);
	i = read_eeprom(i_address);
	d = read_eeprom(d_address);*/
	s = read_eeprom(s_address);
	//p = 15;
	//i = 0.005;
	//d = 0.05;
	//write_eeprom(i, i_address);
	//write_eeprom(d, d_address);
	//write_eeprom(s, s_address);


	

	myPID.setBangBang(4);
	myPID.setTimeStep(1000);
	swichTime = 0;

}

void loop() {
	//delay(1);

	if ((millis() - co) > 1000)
	{
		Serial.print("Time: ");
		Serial.print(millis() / 1000);
		Serial.print("  -   Switch:  ");
		Serial.println(swichTime / 1000);
		co = millis();
	}
	co++;

	updateTemperature();
	menu_control();
	myPID.run();
	digitalWrite(FINE_OUT, !myPID.atSetPoint(0.5));

	lcd.setCursor(0, 0);
	lcd.print(t, 3);
	lcd.setCursor(0, 1);
	lcd.print(outputVal, 3);
	lcd.setCursor(8, 0);
	lcd.print(tr, 2);

	if (millis() > swichTime)
	{
		switch (mode)
		{
		case READY:
			if (outputVal > 2) //Heat
			{
				//time = millis();
				swichTime = millis() + (outputVal * 1000);
				mode = HEAT;
			}
			if (outputVal < -5) // Cool
			{
				//time = millis();
				swichTime = millis() + ((abs(outputVal) + 50) * 1000);
				mode = COOL;
			}
			break;
		case REST:
			mode = READY;
			break;
		case HEAT:
			mode = REST;
			//time = 0;
			swichTime = millis() + ((100 - outputVal) * 1000);
			break;
		case COOL:
			mode = REST;
			//time = 0;
			swichTime = millis() + ((100 - abs(outputVal)) * 1000);
			break;
		default:
			mode = REST;
			break;
		}
	}

	if (mode == HEAT && t > s +0.25)
	{
		myPID.reset();
	}
	if (mode == COOL && t < s-0.25)
	{
		myPID.reset();
	}


	if (mode == COOL && outputVal < 0 && outputVal > 3)
	{
		mode = REST;
	}
	if (mode == HEAT && outputVal > 0 && outputVal < 3)
	{
		mode = REST;
	}

	tempControl(mode);



	if (!digitalRead(btn_down) && !digitalRead(btn_up)) {
		s = 15;
		p = 10;
		i = 0.005;
		d = 0.02;


		write_eeprom(p, p_address);
		write_eeprom(i, i_address);
		write_eeprom(d, d_address);
		write_eeprom(s, s_address);
		delay(1000);
	}
}
void menu_control() {
	print_menu();
	if (!digitalRead(btn_menu))
	{
		delay(10);
		if (!digitalRead(btn_menu))
		{
			pos_menu++;
			if (pos_menu > sizeof(menu) - 1)pos_menu = 0;
			print_menu();
			delay(100);
		}
	}
	else if (!digitalRead(btn_up))
	{
		delay(10);
		if (!digitalRead(btn_up))
		{
			switch (menu[pos_menu])
			{
			case 'p':
				p += 0.05;
				//print_menu();
				write_eeprom(p, p_address);
				break;
			case 'i':
				i += 0.001;
				//print_menu();

				write_eeprom(i, i_address);
				break;
			case 'd':
				d += 0.001;
				//print_menu();

				write_eeprom(d, d_address);
				break;
			case 's':
				s += 0.5;
				print_menu();

				write_eeprom(s, s_address);
				break;

			default:
				break;
			}
			delay(50);

		}
	}
	else if (!digitalRead(btn_down))
	{
		delay(10);
		if (!digitalRead(btn_down))
		{
			switch (menu[pos_menu])
			{
			case 'p':
				p -= 0.05;
			//	print_menu();
				write_eeprom(p, p_address);

				break;
			case 'i':
				i -= 0.001;
			//	print_menu();
				write_eeprom(i, i_address);
				break;
			case 'd':
				d -= 0.001;
			//	print_menu();
				write_eeprom(d, d_address);
				break;
			case 's':
				s -= 0.5;
				print_menu();
				write_eeprom(s, s_address);
				break;

			default:
				break;
			}
			delay(50);

		}
	}
}
void print_menu() {
	lcd.setCursor(9, 1);
	lcd.print(menu[pos_menu]);

	switch (menu[pos_menu])
	{
	case 'p':
		lcd.print(p, 3);
		break;
	case 'i':
		lcd.print(i, 3);
		break;
	case 'd':
		lcd.print(d, 3);
		break;
	case 's':
		lcd.print(s, 3);
		break;
	default:
		break;
	}
}
double read_eeprom(int address)
{
	Serial.println("READ:");

	long two = EEPROM.read(address);
	long one = EEPROM.read(address + 1);
	int value = ((two << 0) & 0xFFFFFF) + ((one << 8) & 0xFFFFFFFF);
	Serial.println(value);
	return value / 1000.0;
}
void write_eeprom(double valueD, int address) {
	Serial.println("Write:");
	int value = (int)(valueD * 1000);
	int16_t data = value * 1000;
	Serial.println(value);

	byte two = (value & 0xFF);
	byte one = ((value >> 8) & 0xFF);

	EEPROM.update(address, two);
	EEPROM.update(address + 1, one);

//	myPID.reset();
	//time = 0;
	//swichTime = 10;
}
bool updateTemperature() {
	if ((millis() - lastTempUpdate) > TEMP_READ_DELAY) {
		t = sensors.getTempC(TEMP_SENS_BEER); //get temp reading
		tr = sensors.getTempC(TEMP_SENS_ROOM);
		//Serial.print(temp);
		//Serial.print("   ");

		//temp2 = temp;
		lastTempUpdate = millis();
		sensors.requestTemperatures(); //request reading for next time
		return true;
	}
	return false;
}//void updateTemperature
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
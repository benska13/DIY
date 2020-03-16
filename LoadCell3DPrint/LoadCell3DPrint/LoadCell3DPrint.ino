int analogInPin = A3;

int sensorValue = 0;

int outputValue = 0;

int transistorPin = 0;

void setup()

{

//Serial.begin(9600);


pinMode(analogInPin, INPUT);
pinMode(transistorPin, OUTPUT);

}

void loop()

{

//sensorValue = analogRead(analogInPin)/4;

//outputValue = map(sensorValue, 0, 255, 0, 255);

//Serial.println(outputValue);

analogWrite(transistorPin, analogRead(analogInPin)/4);
//sensorValue++;
//if (sensorValue > 255) sensorValue=0;

delay(10); }

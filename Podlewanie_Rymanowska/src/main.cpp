#include <Arduino.h>

//#define BLYNK_DEBUG			//Optional, this enables lots of prints
//#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "XXXX";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "XXXX";
char pass[] = "XXXX";


//Definicja pinów
const int SoilMoistureSensorPower_A= D2;	//Odczyt wilgotności gleby z czujnika A, Zasilanie czujnika gdy pin ma wartość HIGH
const int WaterPumpPower_A = D6;		//Załączenie pompy wody A gdy pin ma wartość HIGH

const int SoilMoistureSensorPower_B = D1;	//Odczyt wilgotności gleby z czujnika B, Zasilanie czujnika gdy pin ma wartość HIGH
const int WaterPumpPower_B = D5;		//Załączenie pompy wody B gdy pin ma wartość HIGH

float Humidity_A =0;
float Humidity_B =0;

int Prog_wilgotnosci = 75;
int Check = 0 ;

//Definicja stałych
const unsigned long SleepTime = 10000 * 60 * 60;	//Wejście w stan DeepSleep na 1-dną godzinę (3600000000)
const int PumpTime = 1000 * 20;				//Załączenie pompy na 20s


// This function will run every time Blynk connection is established
BLYNK_CONNECTED() {
  // Request Blynk server to re-send latest values for all pins
  Blynk.syncAll();
}

void Wyslij_Dane()			//Wysyła dane na serwer Blynk
{
	Blynk.virtualWrite(V0, Humidity_A);		//Temperatura [°C]
	Blynk.virtualWrite(V1, Humidity_B);		//Wilgotność [%]
}


BLYNK_WRITE(V10)			//Ustawienie progu wilgotności
{
	Prog_wilgotnosci = param.asInt();
}

float CheckSoilMoisture(int SoilMoistureSensor)	//Sprawdzenie poziomu wilgotności gleby
{
	digitalWrite(SoilMoistureSensor, HIGH);	//Załączenie zasilania czujnika wilgotności
	delay(100);
	int i;
	float sval = 0;
	for (i = 0; i < 5; i++)			//Uśrednianie wartości z czujnika analogowego
	{
		sval = sval + analogRead(A0);	//sensor on analog pin
		delay(10);
	}
	sval = sval / 5;
	digitalWrite(SoilMoistureSensor, LOW);	//Wyłączenie zasilania czujnika wilgotności
	return sval;
}

void Watering()
{
	if (Humidity_A < Prog_wilgotnosci)
	{
		digitalWrite(WaterPumpPower_A, LOW); //Włączenie pompy
		digitalWrite(2, LOW);
		Serial.println("Pompa A załączona");
		delay(PumpTime);
		digitalWrite(2, HIGH);
		Serial.println("Pompa A wyłączona");
		digitalWrite(WaterPumpPower_A, HIGH);  //Wyłączenie pompy
		Blynk.notify("Avocado wysokie zostało podlane :)");
	}

	delay(100);

	if (Humidity_B < Prog_wilgotnosci)
	{
		digitalWrite(WaterPumpPower_B, LOW); //Włączenie pompy
		digitalWrite(2, LOW);
		Serial.println("Pompa B załączona");
		delay(PumpTime);
		digitalWrite(2, HIGH);
		Serial.println("Pompa B wyłączona");
		digitalWrite(WaterPumpPower_B, LOW);  //Wyłączenie pompy
		Blynk.notify("Avocado szerokie zostało podlane :)");
	}

	delay(100);
}

void setup(){
	Blynk.begin(auth, ssid, pass);
	Blynk.run();	
	//Deklaracja Pinów
	pinMode(WaterPumpPower_A, OUTPUT);
	digitalWrite(WaterPumpPower_A, HIGH);
	pinMode(WaterPumpPower_B, OUTPUT);
	digitalWrite(WaterPumpPower_B, HIGH);
	pinMode(SoilMoistureSensorPower_A, OUTPUT);
	pinMode(SoilMoistureSensorPower_B, OUTPUT);
	pinMode(2, OUTPUT);

	Blynk.syncVirtual(V10);

	Serial.begin(115200);
	while (!Serial)
	{
		 // wait for serial port to connect. Needed for native USB
  	}
	delay(100);
	
	Serial.println("Próg wilgotności = " + String(Prog_wilgotnosci));

//############################################################33
	Humidity_A = CheckSoilMoisture(SoilMoistureSensorPower_A);  //Odczyt wilgotności gleby z czujnika A
	Humidity_A = map(Humidity_A, 790, 466, 0, 100); //Convert to Relative Humidity in % (818 -> sensor in air, 427 -> sensor in water)
	Humidity_A = constrain(Humidity_A, 0, 100);   //Limits range of sensor values to between 0 and 100
	Humidity_B = CheckSoilMoisture(SoilMoistureSensorPower_B); //Odczyt wilgotności gleby z czujnika B
	Humidity_B = map(Humidity_B, 800, 450, 0, 100); //Convert to Relative Humidity in % (800 -> sensor in air, 427 -> sensor in water)
	Humidity_B = constrain(Humidity_B, 0, 100);   //Limits range of sensor values to between 0 and 100
	Watering();
	Wyslij_Dane();
//#############################################################33
	//Serial.println("Going to sleep now");
	delay(100);
	Serial.flush();
	
	// Deep sleep mode for 10 seconds, the ESP8266 wakes up by itself when GPIO 16 (D0 in NodeMCU board) is connected to the RESET pin
	//Serial.println("I'm awake, but I'm going into deep sleep mode for 10 seconds");
	ESP.deepSleep(3600e6);  //60e6 = 1min, 3600e6 = 1h
}


void loop()
{
	//This is not going to be called

	Humidity_A = CheckSoilMoisture(SoilMoistureSensorPower_A);  //Odczyt wilgotności gleby z czujnika A
	Humidity_A = map(Humidity_A, 812, 456, 0, 100); //Convert to Relative Humidity in % (818 -> sensor in air, 427 -> sensor in water)
	Humidity_A = constrain(Humidity_A, 0, 100);   //Limits range of sensor values to between 0 and 100
	Humidity_B = CheckSoilMoisture(SoilMoistureSensorPower_B); //Odczyt wilgotności gleby z czujnika B
	Humidity_B = map(Humidity_B, 812, 456, 0, 100); //Convert to Relative Humidity in % (818 -> sensor in air, 427 -> sensor in water)
	Humidity_B = constrain(Humidity_B, 0, 100);   //Limits range of sensor values to between 0 and 100

	Serial.println("Humidity_A: " + String(Humidity_A) +"%, " + String(CheckSoilMoisture(SoilMoistureSensorPower_A)));
	Serial.println("Humidity_B: " + String(Humidity_B) +"%, " + String(CheckSoilMoisture(SoilMoistureSensorPower_B)));
	Serial.println("");

	delay(100);
}


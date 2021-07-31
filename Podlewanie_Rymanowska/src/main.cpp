#include <Arduino.h>

//#define BLYNK_DEBUG				//Optional, this enables lots of prints
//#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#include <SimpleTimer.h>			//https://github.com/jfturcot/SimpleTimer
SimpleTimer Timer;				//Timer do sprawdzania połaczenia z BLYNKiem (co 30s) i uruchamiania MainFunction (co 3s)

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "7ee4a4fee7f74a7596a37458bdef6168";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "ECN";
char pass[] = "Pecherek1987";


//Definicja pinów
const int SoilMoistureSensorPower_A= D2;		//Odczyt wilgotności gleby z czujnika A, Zasilanie czujnika gdy pin ma wartość HIGH
const int WaterPumpPower_A = D6;			//Załączenie pompy wody A gdy pin ma wartość HIGH

const int SoilMoistureSensorPower_B = D1;		//Odczyt wilgotności gleby z czujnika B, Zasilanie czujnika gdy pin ma wartość HIGH
const int WaterPumpPower_B = D5;			//Załączenie pompy wody B gdy pin ma wartość HIGH

float Humidity_A = 0;					//Zmierzona wartość wilgotności z czujnika A
float Humidity_B = 0;					//Zmierzona wartość wilgotności z czujnika B

int Prog_wilgotnosciA = 85;				//Wilgotność poniżej której będzie podlewał roślinę A (domyślnie 85%)
int Prog_wilgotnosciB = 85;				//Wilgotność poniżej której będzie podlewał roślinę B (domyślnie 85%)
int Check = 0 ;

//Definicja stałych
const unsigned long SleepTime = 10000 * 60 * 60;	//Wejście w stan DeepSleep na 1-dną godzinę (3600000000)
const int PumpTimeKaki = 1000 * 30;			//Załączenie pompy na 10s
const int PumpTimeMango = 1000 * 45;			//Załączenie pompy na 10s

//---------------------------------------------------------------------------------------------------------------------------------------------

//Wysyłanie powiadomień przez Blynk.notify() gdy będzie podlewanie
void MoistureNotification()
{
	if (Humidity_A < Prog_wilgotnosciA && Humidity_B < Prog_wilgotnosciB)
	{
		Blynk.notify("Obie rośliny zostały podlane :)");
	}
	else if (Humidity_B < Prog_wilgotnosciB)
	{
		Blynk.notify("Kaki zostało podlane :)");
	}
	else if (Humidity_A < Prog_wilgotnosciA)
	{
		Blynk.notify("Mango zostało podlane :)");
	}
}

//This function will run every time Blynk connection is established
BLYNK_CONNECTED()
{
	Blynk.syncAll();				// Request Blynk server to re-send latest values for all pins
}

//Wysyła dane na serwer Blynk
void Wyslij_Dane()
{
	Blynk.virtualWrite(V0, Humidity_A);		//Wilgotność [%]
	Blynk.virtualWrite(V1, Humidity_B);		//Wilgotność [%]
	Blynk.virtualWrite(V2, Prog_wilgotnosciA);	//Prog_wilgotnosci dla rośliny A [%]
	Blynk.virtualWrite(V3, Prog_wilgotnosciB);	//Prog_wilgotnosci dla rośliny B [%]
}

//Ustawienie progu wilgotności dla rośliny a
BLYNK_WRITE(V10)
{
	Prog_wilgotnosciA = param.asInt();
	//Prog_wilgotnosciB = Prog_wilgotnosciA;	//Tymczasowe spięcie, bo nie ma kredytó na konie Blynk, w przyszłości zostanie to rozdzielone
}

//Ustawienie progu wilgotności dla rośliny B
BLYNK_WRITE(V11)
{
	Prog_wilgotnosciB = param.asInt();
}

//Działą jak map() ale zwraca liczby rzeczywiste a nie tylko całkowite
double mapf(double val, double in_min, double in_max, double out_min, double out_max)
{
    return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//Sprawdzenie poziomu wilgotności gleby
void CheckSoilMoisture(int SoilMoistureSensor)
{
	digitalWrite(SoilMoistureSensor, HIGH);		//Załączenie zasilania czujnika wilgotności
	delay(100);
	int i;
	double sval = 0;
	for (i = 0; i < 5; i++)				//Uśrednianie wartości z czujnika analogowego
	{
		sval = sval + analogRead(A0);		//sensor on analog pin
		delay(10);
	}
	sval = sval / 5;
	digitalWrite(SoilMoistureSensor, LOW);		//Wyłączenie zasilania czujnika wilgotności


	if (SoilMoistureSensor == SoilMoistureSensorPower_A)
	{
		Humidity_A = mapf(sval, 729, 420, 0, 100); 		//Convert to Relative Humidity in % (818 -> sensor in air, 427 -> sensor in water)
		//Humidity_A = constrain(Humidity_A, 0, 100);		//Limits range of sensor values to between 0 and 100
		Serial.print("A row value = ");
		Serial.print(sval);
		Serial.print("   Humidity% = ");
		Serial.println(Humidity_A);

	}
	else if (SoilMoistureSensor == SoilMoistureSensorPower_B)
	{
		Humidity_B = mapf(sval, 774, 436, 0, 100);		//Convert to Relative Humidity in % (800 -> sensor in air, 427 -> sensor in water)
		//Humidity_B = constrain(Humidity_B, 0, 100);		//Limits range of sensor values to between 0 and 100
		Serial.print("B row value = ");
		Serial.print(sval);
		Serial.print("   Humidity% = ");
		Serial.println(Humidity_B);
	}
}

//Załączenie pompy gdy wilgotność poniżej progu
void Watering()
{
	if (Humidity_A < Prog_wilgotnosciA)
	{
		digitalWrite(WaterPumpPower_A, LOW); //Włączenie pompy
		digitalWrite(2, LOW);
		Serial.println("Pompa A załączona");
		delay(PumpTimeMango);
		digitalWrite(2, HIGH);
		Serial.println("Pompa A wyłączona");
		digitalWrite(WaterPumpPower_A, HIGH);  //Wyłączenie pompy
	}

	delay(100);

	if (Humidity_B < Prog_wilgotnosciB)
	{
		digitalWrite(WaterPumpPower_B, LOW); //Włączenie pompy
		digitalWrite(2, LOW);
		Serial.println("Pompa B załączona");
		delay(PumpTimeKaki);
		digitalWrite(2, HIGH);
		Serial.println("Pompa B wyłączona");
		digitalWrite(WaterPumpPower_B, HIGH);  //Wyłączenie pompy
	}
}

//Prześjście w stan DeepSleep po 30s
void GoToSleep()
{
	// Deep sleep mode for 30 seconds, the ESP8266 wakes up by itself when GPIO 16 (D0 in NodeMCU board) is connected to the RESET pin
	ESP.deepSleep(3600e6);  //60e6 = 1min, 3600e6 = 1h
}

//Uruchamia po kolei wszystkie niezbędne funcje
void MainAction()
{
	CheckSoilMoisture(SoilMoistureSensorPower_A);			//Odczyt wilgotności gleby z czujnika A [%]
	CheckSoilMoisture(SoilMoistureSensorPower_B);			//Odczyt wilgotności gleby z czujnika B [%]
	Wyslij_Dane();							//Wysyła dane na serwer Blynk
	MoistureNotification();						//Wysyłanie powiadomień przez Blynk.notify() gdy będzie podlewanie
	Watering();							//Załączenie pompy gdy wilgotność poniżej progu
	//Zostanie uruchomione tylko raz za 30s, potrzebne aby dać czas połączyć się z Blynkiem i wymienić informacje
	Timer.setTimeout(30000L, GoToSleep);
}

//---------------------------------------------------------------------------------------------------------------------------------------------

void setup()
{
	Blynk.begin(auth, ssid, pass);
	//Istawienie zadania które zostanie uruchomione tylko raz po 1s od Timer.run();
	Timer.setTimeout(5000L, MainAction);

	//Deklaracja Pinów
	pinMode(WaterPumpPower_A, OUTPUT);				//Zasilanie pompy A
	digitalWrite(WaterPumpPower_A, HIGH);				//Ustawienie wartośći HIGH = pompa wyłączona
	pinMode(WaterPumpPower_B, OUTPUT);				//Zasilanie pompy B
	digitalWrite(WaterPumpPower_B, HIGH);				//Ustawienie wartośći HIGH = pompa wyłączona
	pinMode(SoilMoistureSensorPower_A, OUTPUT);			//Zasilanie czujnika wilgotności A
	pinMode(SoilMoistureSensorPower_B, OUTPUT);			//Zasilanie czujnika wilgotności B
	pinMode(2, OUTPUT);						//Dioda LED

	Serial.begin(115200);

	while (!Serial)
	{
		 // wait for serial port to connect. Needed for native USB
  	}
}

void loop()
{
	if (Blynk.connected()) Blynk.run();
	
	Timer.run();
/*
	CheckSoilMoisture(SoilMoistureSensorPower_A);			//Odczyt wilgotności gleby z czujnika A [%]
	CheckSoilMoisture(SoilMoistureSensorPower_B);			//Odczyt wilgotności gleby z czujnika B [%]
	Serial.println("");
	Serial.println("");
	delay(1000);
*/

}
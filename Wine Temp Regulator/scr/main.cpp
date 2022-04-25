#include <Arduino.h>

//Biblioteki potrzebne dla AutoConnect
#include <WiFi.h>				// Replace 'ESP8266WiFi.h' with 'WiFi.h. for ESP32
#include <WebServer.h>			// Replace 'ESP8266WebServer.h'with 'WebServer.h' for ESP32
#include <AutoConnect.h>
WebServer			Server;		// Replace 'ESP8266WebServer' with 'WebServer' for ESP32
AutoConnect			Portal(Server);
AutoConnectConfig	Config;		// Enable autoReconnect supported on v0.9.4


//#define BLYNK_DEBUG				// Optional, this enables lots of prints
/* Comment this out to disable prints and save space */
//#define BLYNK_PRINT Serial
//#include <WiFi.h> //alreadydeclared for AutoConnect
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>		// Replace 'BlynkSimpleEsp8266' with 'BlynkSimpleEsp32.h. for ESP32
#include <WidgetRTC.h>
WidgetTerminal	terminal(V40);		// Attach virtual serial terminal to Virtual Pin V40
WidgetLED		BlynkLED1(V11);		// Inicjacja diody LED dla załączania przekaźnika
WidgetLED		BlynkLED2(V12);		// Inicjacja diody LED dla załączania przekaźnika
WidgetLED		BlynkLED3(V13);		// Inicjacja diody LED dla załączania przekaźnika
WidgetLED		BlynkLED4(V14);		// Inicjacja diody LED dla załączania przekaźnika
//WidgetRTC		rtc;				// Inicjacja widgetu zegara czasu rzeczywistego RTC

//SHT31
#include <EnvironmentCalculations.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"
Adafruit_SHT31 sht31 = Adafruit_SHT31();

#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 4				// Data wire is plugged into pin 4 on the ESP32
#define TEMPERATURE_PRECISION 12	// Ustawienie precyzji z jaką odczytywana jest temperatura
/* The datasheet states conversion time takes max: (from my head)
12 bit ; 750 millisec ; 0.0625°C
11 bit ; 375 millisec ; 0.125°C
10 bit ; 187 millisec ; 0.25°C
9 bit  ; 94  millisec ; 0.5°C */

OneWire oneWire(ONE_WIRE_BUS);			// Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);	// Pass our oneWire reference to Dallas Temperature.
// variable to hold device addresses
// DeviceAddress Thermometer;
// int deviceCount = 0;
// Deklaracja adresów czujników DS18B20
DeviceAddress sensor1a = { 0x28, 0x7C, 0x86, 0x75, 0xD0, 0x01, 0x3C, 0x45 };	// Adres czujnika #1a
DeviceAddress sensor1b = { 0x28, 0xFF, 0x64, 0x18, 0xD0, 0x6E, 0x9C, 0x4F };	// Adres czujnika #1b
DeviceAddress sensor2a = { 0x28, 0x41, 0x31, 0x75, 0xD0, 0x01, 0x3C, 0xFC };	// Adres czujnika #2a
DeviceAddress sensor2b = { 0x28, 0x09, 0x0C, 0x75, 0xD0, 0x01, 0x3C, 0x40 };	// Adres czujnika #2b
DeviceAddress sensor3a = { 0x28, 0x84, 0x1A, 0x75, 0xD0, 0x01, 0x3C, 0x00 };	// Adres czujnika #3a
DeviceAddress sensor3b = { 0x28, 0x49, 0x83, 0x76, 0xE0, 0x01, 0x3C, 0xC7 };	// Adres czujnika #3b
DeviceAddress sensor4a = { 0x28, 0xC8, 0xEC, 0x75, 0xD0, 0x01, 0x3C, 0x32 };	// Adres czujnika #4a
DeviceAddress sensor4b = { 0x28, 0xE5, 0x20, 0x75, 0xD0, 0x01, 0x3C, 0xCD };	// Adres czujnika #4b

// Definicja pinów sterujących przekaźnikami
#define		WineRelay_1	 12				// Numer pinu dla przekaźnika sterującego grzałką wina #1 [HIGH / LOW]
#define		WineRelay_2	 25				// Numer pinu dla przekaźnika sterującego grzałką wina #2 [HIGH / LOW]
#define		WineRelay_3	 26				// Numer pinu dla przekaźnika sterującego grzałką wina #3 [HIGH / LOW]
#define		WineRelay_4	 27				// Numer pinu dla przekaźnika sterującego grzałką wina #4 [HIGH / LOW]

#define		SDA_Pin		 33				// Numer pinu SDA dla czujnika SHT31
#define		SCI_Pin		 32				// Numer pinu SCI dla czujnika SHT31

// Deklaracja zmiennych przechowujących wartości aktualnych temperatur zmierzonych z czujników DS18B20
float		WineTemp_1a	= 0;			// Zmienna przechowująca temperaturę dla wina #1a [°C]
float		WineTemp_1b	= 0;			// Zmienna przechowująca temperaturę dla wina #1b [°C]
float		WineTemp_2a	= 0;			// Zmienna przechowująca temperaturę dla wina #2a [°C]
float		WineTemp_2b	= 0;			// Zmienna przechowująca temperaturę dla wina #2b [°C]
float		WineTemp_3a	= 0;			// Zmienna przechowująca temperaturę dla wina #3a [°C]
float		WineTemp_3b	= 0;			// Zmienna przechowująca temperaturę dla wina #3b [°C]
float		WineTemp_4a	= 0;			// Zmienna przechowująca temperaturę dla wina #4a [°C]
float		WineTemp_4b	= 0;			// Zmienna przechowująca temperaturę dla wina #4b [°C]

// Deklaracja zmiennych przechowujących wartości temperatur jakich zarządał urzutkownik na konkretnym nastawie wina
float		SetTemp_1	= 10.6;			// Zmienna przechowująca temperaturę dla wina #1 [°C]
float		SetTemp_2	= 12.5;			// Zmienna przechowująca temperaturę dla wina #2 [°C]
float		SetTemp_3	= 11.5;			// Zmienna przechowująca temperaturę dla wina #3 [°C]
float		SetTemp_4	= 12.1;			// Zmienna przechowująca temperaturę dla wina #4 [°C]
const float	TemtHist	= 0.6;			// histereza dla temperatury

// Deklaracja zmiennych przechowujących informację binarną TRUE, FALSE czy mata grzeje czy nie.
bool		HeatingOn_1	= false;		// Czy mata #1 grzeje (true / false)
bool		HeatingOn_2	= false;		// Czy mata #2 grzeje (true / false)
bool		HeatingOn_3	= false;		// Czy mata #3 grzeje (true / false)
bool		HeatingOn_4	= false;		// Czy mata #4 grzeje (true / false)

// Deklaracja zmiennych przechowujących wartości z czujnika SHT31 (temperatura i wilgotność otoczenia)
float		SHT31_Temp	= 0;			// Zmienna przechowująca temperaturę powietrza [°C]
float		SHT31_Humid	= 0;			// Zmienna przechowująca wilgotność powietrz [°C]
// Dane obliczane na podstawie danych z czujnika
float		dewPoint	= 0;			// Temperatura punktu rosy [°C]
float		absHum		= 0;			// Wilgotność bezwzględna [g/m^3]
float		heatIndex	= 0;			// Temperatura odczuwalna [°C]
bool		CtrButton1	= false;		// Czy ma kontrolować matę grzejną w wino #1 (true / false)
bool 		CtrButton2	= false;		// Czy ma kontrolować matę grzejną w wino #2 (true / false)
bool 		CtrButton3	= false;		// Czy ma kontrolować matę grzejną w wino #3 (true / false)
bool 		CtrButton4	= false;		// Czy ma kontrolować matę grzejną w wino #4 (true / false)
bool		RestartESP	= false;		// Gdy true i w terminalu YES wykona restart

#include <SimpleTimer.h>
SimpleTimer	timer;						// the timer object
static volatile int timerID	= -1;		//Przetrzymuje ID Timera https://desire.giesecke.tk/index.php/2018/01/30/change-global-variables-from-isr/
int			timerIDReset	= -1;		//Przetrzymuje ID Timera https://desire.giesecke.tk/index.php/2018/01/30/change-global-variables-from-isr/

//STAŁE
const char	auth[]		= "wwhiH-ZuRi2EFLNW85CoGnu0_TlVcx_j";	// Token Pokój Rymanowska
const char	server[]	= "stecko.duckdns.org";					// Adres IP serwera na którym jest zainstalowany server BLYNK lub stecko.duckdns.org
const int	port		= 8080;									// Port na którym jest serwer Blykn

// Sterowanie przekaźnikami do kontroli temperatury wina
void Wine_Temp_Control()
{
	// Regulacja temperatury wina #1
	if (CtrButton1)
	{
		if (WineTemp_1b < SetTemp_1 - TemtHist)
		{
			digitalWrite(WineRelay_1, LOW);	// Wysłanie sygnału do włączenia maty grzejnej
			BlynkLED1.on();						// Dioda Blynk włączona
		}
		else if (WineTemp_1b > SetTemp_1 + TemtHist )
		{
			digitalWrite(WineRelay_1, HIGH);		// Wysłanie sygnału do wyłączenia maty grzejnej
			BlynkLED1.off();					// Dioda Blynk wyłączona
		}
	}

	// Regulacja temperatury wina #2
	if (CtrButton2)
	{
		if (WineTemp_2b < SetTemp_2 - TemtHist )
		{
			digitalWrite(WineRelay_2, LOW);	// Wysłanie sygnału do włączenia maty grzejnej
			BlynkLED2.on();						// Dioda Blynk włączona
		}
		else if (WineTemp_2b > SetTemp_2 + TemtHist )
		{
			digitalWrite(WineRelay_2, HIGH);		// Wysłanie sygnału do wyłączenia maty grzejnej
			BlynkLED2.off();					// Dioda Blynk wyłączona
		}
	}

	// Regulacja temperatury wina #3
	if (CtrButton3)
	{
		if (WineTemp_3b < SetTemp_3 - TemtHist )
		{
			digitalWrite(WineRelay_3, LOW);	// Wysłanie sygnału do włączenia maty grzejnej
			BlynkLED3.on();						// Dioda Blynk włączona
		}
		else if (WineTemp_3b > SetTemp_3 + TemtHist )
		{
			digitalWrite(WineRelay_3, HIGH);		// Wysłanie sygnału do wyłączenia maty grzejnej
			BlynkLED3.off();					// Dioda Blynk wyłączona
		}
	}
	// Regulacja temperatury wina #4
	if (CtrButton4)
	{
		if (WineTemp_4b < SetTemp_4 - TemtHist )
		{
			digitalWrite(WineRelay_4, LOW);	// Wysłanie sygnału do włączenia maty grzejnej
			BlynkLED4.on();						// Dioda Blynk włączona
		}
		else if (WineTemp_4b > SetTemp_4 + TemtHist )
		{
			digitalWrite(WineRelay_4, HIGH);		// Wysłanie sygnału do wyłączenia maty grzejnej
			BlynkLED4.off();					// Dioda Blynk wyłączona
		}
	}
}

// Informacja że połączono z serwerem Blynk, synchronizacja danych
BLYNK_CONNECTED()
{
	Serial.println("Reconnected, syncing with cloud.");
	//rtc.begin();
	//Blynk.syncAll();			// Zamiast synchronizować wszystkie możliwe piny, lepiej tylko te które są wymagana (optymalizacja)
	Blynk.syncVirtual(V21, V22, V23, V24, V31, V32, V33, V34);
	Wine_Temp_Control();		// Wywołanie funkcji sterującej regulacją temperatury
	/*
	Blynk.syncVirtual(V7);		// Przycisk w aplikacji Blynk On / Off kontrolujący czy ma kontrolować matę grzejną w wino #1
	Blynk.syncVirtual(V8);		// Przycisk w aplikacji Blynk On / Off kontrolujący czy ma kontrolować matę grzejną w wino #2
	Blynk.syncVirtual(V9);		// Przycisk w aplikacji Blynk On / Off kontrolujący czy ma kontrolować matę grzejną w wino #3
	Blynk.syncVirtual(V10);		// Przycisk w aplikacji Blynk On / Off kontrolujący czy ma kontrolować matę grzejną w wino #4
	Blynk.syncVirtual(V21);		// Ustawienie progu temperatury poniżej której załączy się grzałka na winie #1
	Blynk.syncVirtual(V22);		// Ustawienie progu temperatury poniżej której załączy się grzałka na winie #2
	Blynk.syncVirtual(V23);		// Ustawienie progu temperatury poniżej której załączy się grzałka na winie #3
	Blynk.syncVirtual(V24);		// Ustawienie progu temperatury poniżej której załączy się grzałka na winie #4
	*/
}

// Sprawdza czy połączone z serwerem Blynk
void blynkCheck()
{
	if (WiFi.status() == WL_CONNECTED)		//WL_CONNECTED: assigned when connected to a WiFi network
	{
		if (!Blynk.connected())
		{
			Serial.println("WiFi OK, trying to connect to the Blynk server...");
			Blynk.connect();
		}
	}

	if (WiFi.status() == WL_NO_SSID_AVAIL)		//WL_NO_SSID_AVAIL: assigned when no SSID are available
	{
		Serial.println("No WiFi connection, offline mode.");
	}

	if (WiFi.status() == WL_IDLE_STATUS)		//WL_IDLE_STATUS is a temporary status assigned when WiFi.begin() is called and remains active until the number of attempts expires (resulting in WL_CONNECT_FAILED) or a connection is established (resulting in WL_CONNECTED)
	{
		Serial.println("WL_IDLE_STATUS: WiFi.begin() is called");
	}

	if (WiFi.status() == WL_SCAN_COMPLETED)		//WL_SCAN_COMPLETED: assigned when the scan networks is completed
	{
		Serial.println("WL_SCAN_COMPLETED: networks is completed");
	}

	if (WiFi.status() == WL_CONNECT_FAILED)		//WL_CONNECT_FAILED: assigned when the connection fails for all the attempts
	{
		Serial.println("WL_CONNECT_FAILED: connection fails for all the attempts");
	}

	if (WiFi.status() == WL_CONNECTION_LOST)	//WL_CONNECTION_LOST: assigned when the connection is lost
	{
		Serial.println("WL_CONNECTION_LOST: the connection is lost");
	}

	if (WiFi.status() == WL_DISCONNECTED)		//WL_DISCONNECTED: assigned when disconnected from a network
	{
		Serial.println("WL_DISCONNECTED: disconnected from a network");
	}
}

// Odczyt wskazań z czujnika SHT31
void Read_SHT31_Values()
{
	SHT31_Temp	= sht31.readTemperature() -2.5;		//Korekta dla temperatury dla czujnika SHT31
	SHT31_Humid	= sht31.readHumidity() + 5;		//Korekta poziomu wilgotności dla czujnika SHT31
	Serial.print("SHT31 Temp: ");
	Serial.println(SHT31_Temp);
	EnvironmentCalculations::TempUnit     envTempUnit =  EnvironmentCalculations::TempUnit_Celsius;
	//Dane obliczane na podstawie danych z czujnika
	dewPoint	= EnvironmentCalculations::DewPoint(SHT31_Temp, SHT31_Humid, envTempUnit);
	absHum		= EnvironmentCalculations::AbsoluteHumidity(SHT31_Temp, SHT31_Humid, envTempUnit);
	heatIndex	= EnvironmentCalculations::HeatIndex(SHT31_Temp, SHT31_Humid, envTempUnit);
}

// Odczyt temperatury (funkcja wywoływana co 5s)
void ReadTemperaraturesFromDS18B20()
{
	sensors.requestTemperatures();
	WineTemp_1a = sensors.getTempC(sensor1a) + 0.58;
	WineTemp_1b = sensors.getTempC(sensor1b) - 0.33;
	WineTemp_2a = sensors.getTempC(sensor2a) - 0.07;
	WineTemp_2b = sensors.getTempC(sensor2b) - 0.52;
	WineTemp_3a = sensors.getTempC(sensor3a) - 0.31;
	WineTemp_3b = sensors.getTempC(sensor3b) + 0.56;
	WineTemp_4a = sensors.getTempC(sensor4a) + 0.30;
	WineTemp_4b = sensors.getTempC(sensor4b) - 0.23;
}

// Zwraca siłę sygnału WiFi sieci do której jest podłączony w %. REF: https://www.adriangranados.com/blog/dbm-to-percent-conversion
int WiFi_Strength (long Signal)
{
	return constrain(round((-0.0154*Signal*Signal)-(0.3794*Signal)+98.182), 0, 100);
}

// signal strength levels https://www.netspotapp.com/what-is-rssi-level.html
String WiFi_levels(long Signal)
{
	if (Signal >= -50)
	{
		return "Excellent";
	}
	else if (Signal < -50 && Signal >= -60)
	{
		return "Very good";
	}
	else if (Signal < -60 && Signal >= -70)
	{
		return "Good";
	}
	else if (Signal < -70 && Signal >= -80)
	{
		return "Low";
	}
	else if (Signal < -80 && Signal >= -90)
	{
		return "Very low";
	}
	return "Unusable";
}

// Wysyłanie danych na serwer Blynka
void Wyslij_Dane()
{
	Blynk.virtualWrite(V1, WineTemp_1a);		// Temperatura z czujnika DS18B20 z wina #1a [°C]
	Blynk.virtualWrite(V2, WineTemp_1b);		// Temperatura z czujnika DS18B20 z wina #1b [°C]
	Blynk.virtualWrite(V3, WineTemp_2a);		// Temperatura z czujnika DS18B20 z wina #2a [°C]
	Blynk.virtualWrite(V4, WineTemp_2b);		// Temperatura z czujnika DS18B20 z wina #2b [°C]
	Blynk.virtualWrite(V5, WineTemp_3a);		// Temperatura z czujnika DS18B20 z wina #3a [°C]
	Blynk.virtualWrite(V6, WineTemp_3b);		// Temperatura z czujnika DS18B20 z wina #3b [°C]
	Blynk.virtualWrite(V7, WineTemp_4a);		// Temperatura z czujnika DS18B20 z wina #4a [°C]
	Blynk.virtualWrite(V8, WineTemp_4b);		// Temperatura z czujnika DS18B20 z wina #4b [°C]
	Blynk.virtualWrite(V15, SHT31_Temp);		// Temperatura z czujnika SHT31 [°C]
	Blynk.virtualWrite(V16, SHT31_Humid);		// Wilgotność z czujnika SHT31 [%]
	Blynk.virtualWrite(V17, dewPoint);			// Temperatura punktu rosy [°C]
	Blynk.virtualWrite(V18, absHum);			// Wilgotność bezwzględna [g/m^3]
	Blynk.virtualWrite(V19, heatIndex);			// Temperatura odczuwalna [°C]
	Blynk.virtualWrite(V25, WiFi_Strength(WiFi.RSSI()));	// Siła sygnału Wi-Fi [%], constrain() limits range of sensor values to between 0 and 100
	Blynk.virtualWrite(V41, HeatingOn_1);		// Czy mata #1 grzeje (true / false)
	Blynk.virtualWrite(V42, HeatingOn_2);		// Czy mata #2 grzeje (true / false)
	Blynk.virtualWrite(V43, HeatingOn_3);		// Czy mata #3 grzeje (true / false)
	Blynk.virtualWrite(V44, HeatingOn_4);		// Czy mata #4 grzeje (true / false)
	Blynk.virtualWrite(V51, SetTemp_1);			// Zmienna przechowująca temperaturę dla wina #1 [°C]
	Blynk.virtualWrite(V52, SetTemp_2);			// Zmienna przechowująca temperaturę dla wina #2 [°C]
	Blynk.virtualWrite(V53, SetTemp_3);			// Zmienna przechowująca temperaturę dla wina #3 [°C]
	Blynk.virtualWrite(V54, SetTemp_4);			// Zmienna przechowująca temperaturę dla wina #4 [°C]
}

// Wyłącza możliwość restartu ESP (timer ustawiony na 30s)
void RestartCounter()
{
	RestartESP = false;	// Gdy true i w terminalu YES wykona restart
}

// Soft restart sterownika
void RestartESP32()
{
	//terminal.clear();
	//terminal.println("Restarting!");
	//terminal.flush();
	//delay(1000);
	ESP.restart(); 	//Restartuje sterownik
}

// Przycisk w aplikacji Blynk On / Off kontrolujący czy ma kontrolować matę grzejną w wino #1
BLYNK_WRITE(V31)
{  // Button Widget with set range of 0-1
	if (param.asInt())
	{
		CtrButton1	= true;							// Display Widget output if param 1
		Blynk.setProperty(V21,"color","#ED9D00");	// Pomarańczowy #B97B02
		Wine_Temp_Control(); 	// Uruchomienie funkcji Wine_Temp_Control() aby zadziałało natychmiast
	}
	else
	{
		CtrButton1	= false;						// Display Widget output if param 0
		digitalWrite(WineRelay_1, HIGH);				// Wysłanie sygnału do wyłączenia maty grzejnej
		Blynk.setProperty(V21,"color", "#3A2E18");	// Stłumiony pomarańczowy
		BlynkLED1.off();							// Dioda Blynk wyłączona
	}
}

// Przycisk w aplikacji Blynk On / Off kontrolujący czy ma kontrolować matę grzejną w wino #2
BLYNK_WRITE(V32)
{  // Button Widget with set range of 0-1
	if (param.asInt())
	{
		CtrButton2	= true;							// Display Widget output if param 1
		Blynk.setProperty(V22,"color","#ED9D00");	// Pomarańczowy
		Wine_Temp_Control(); 	// Uruchomienie funkcji Wine_Temp_Control() aby zadziałało natychmiast
	}
	else
	{
		CtrButton2	= false;						// Display Widget output if param 0
		digitalWrite(WineRelay_2, HIGH);				// Wysłanie sygnału do wyłączenia maty grzejnej
		Blynk.setProperty(V22,"color", "#3A2E18");	// Stłumiony pomarańczowy
		BlynkLED2.off();							// Dioda Blynk wyłączona
	}
}

// Przycisk w aplikacji Blynk On / Off kontrolujący czy ma kontrolować matę grzejną w wino #3
BLYNK_WRITE(V33)
{  // Button Widget with set range of 0-1
	if (param.asInt())
	{
		CtrButton3	= true;							// Display Widget output if param 1
		Blynk.setProperty(V23,"color","#ED9D00");	// Pomarańczowy
		Wine_Temp_Control(); 	// Uruchomienie funkcji Wine_Temp_Control() aby zadziałało natychmiast
	}
	else
	{
		CtrButton3	= false;						// Display Widget output if param 0
		digitalWrite(WineRelay_3, HIGH);				// Wysłanie sygnału do wyłączenia maty grzejnej
		Blynk.setProperty(V23,"color", "#3A2E18");	// Stłumiony pomarańczowy
		BlynkLED3.off();							// Dioda Blynk wyłączona
	}
}

// Przycisk w aplikacji Blynk On / Off kontrolujący czy ma kontrolować matę grzejną w wino #4
BLYNK_WRITE(V34)
{  // Button Widget with set range of 0-1
	if (param.asInt())
	{
		CtrButton4	= true;							// Display Widget output if param 1
		Blynk.setProperty(V24,"color","#ED9D00");	// Pomarańczowy
		Wine_Temp_Control(); 	// Uruchomienie funkcji Wine_Temp_Control() aby zadziałało natychmiast
	}
	else
	{
		CtrButton4	= false;						// Display Widget output if param 0
		digitalWrite(WineRelay_4, HIGH);				// Wysłanie sygnału do wyłączenia maty grzejnej
		Blynk.setProperty(V24,"color", "#3A2E18");	// Stłumiony pomarańczowy
		BlynkLED4.off();							// Dioda Blynk wyłączona
	}
}

// Obsługa terminala
BLYNK_WRITE(V40)
{
	String TerminalCommand = param.asStr();
	TerminalCommand.toLowerCase();

	if (String("ports") == TerminalCommand)
	{
		terminal.clear();
		terminal.println("PORT    DESCRIPTION        UNIT");
		terminal.println("V1   <- Wine temp #1       °C");
		terminal.println("V2   <- Wine temp #2       °C");
		terminal.println("V3   <- Wine temp #3       °C");
		terminal.println("V4   <- Wine temp #4       °C");
		terminal.println("V21  -> Set Temp #1        °C");
		terminal.println("V22  -> Set Temp #2        °C");
		terminal.println("V23  -> Set Temp #3        °C");
		terminal.println("V24  -> Set Temp #4        °C");
		terminal.println("V11  -> BlynkLED1          0/1");
		terminal.println("V12  -> BlynkLED2          0/1");
		terminal.println("V13  -> BlynkLED3          0/1");
		terminal.println("V14  -> BlynkLED4          0/1");
		terminal.println("V25  -> WiFi Signal        %");
		terminal.println("V40 <-> Terminal           String");
	}
	else if (String("values") == TerminalCommand)
	{
		terminal.clear();
	      terminal.println("PORT   DATA         VALUE ");
		terminal.print("V1   Wine temp #1 = ");
		terminal.print(WineTemp_1a);
		terminal.println(" °C");
		terminal.print("V2   Wine temp #2 = ");
		terminal.print(WineTemp_2a);
		terminal.println(" °C");
		terminal.print("V3   Wine temp #2 = ");
		terminal.print(WineTemp_3a);
		terminal.println(" °C");
		terminal.print("V4   Wine temp #2 = ");
		terminal.print(WineTemp_4a);
		terminal.println(" °C");
		terminal.print("V21   Set Temp #1 = ");
		terminal.print(SetTemp_1);
		terminal.println(" °C");
		terminal.print("V22   Set Temp #2 = ");
		terminal.print(SetTemp_2);
		terminal.println(" °C");
		terminal.print("V23   Set Temp #3 = ");
		terminal.print(SetTemp_3);
		terminal.println(" °C");
		terminal.print("V24   Set Temp #4 = ");
		terminal.print(SetTemp_4);
		terminal.println(" °C");
		terminal.print("V25  WiFi Signal  = ");
		terminal.print(WiFi_Strength(WiFi.RSSI()));
		terminal.print("%, ");
		terminal.print(WiFi.RSSI());
		terminal.print("dBm, ");
		terminal.println(WiFi_levels(WiFi.RSSI()));
		terminal.print("AutoConnect IP    = ");
		terminal.print(WiFi.localIP().toString() + "/_ac");
	}
	else if (String("constant") == TerminalCommand)
	{
		terminal.clear();
	      terminal.println("CONSTANT      VALUE ");
		terminal.print("TemtHist   = ±");
		terminal.print(TemtHist);
		terminal.println("°C");
	}
	else if (String("restart") == TerminalCommand)
	{
		terminal.clear();
		terminal.println("Are you sure you want to restart?");
		terminal.println("Type YES if you are sure to restart...");
		RestartESP = true;	//Gdy true i w terminalu YES wykona restart
		timer.setTimeout(30000, RestartCounter);
	}
	else if (String("yes") == TerminalCommand)
	{
		if (RestartESP)
		{
			ESP.restart();
		}
	}
	else if (String("hello") == TerminalCommand)
	{
		terminal.clear();
		terminal.println("Hi Łukasz. Have a great day!");
	}
	else if (String("cls") == TerminalCommand)
	{
		terminal.clear();
	}
	else
	{
		terminal.clear();
		terminal.println("Type 'PORTS' to show list");
		terminal.println("Type 'VALUES' to show sensor data");
		terminal.println("Type 'CONSTANT' show constant values");
		terminal.println("Type 'RESTART' to restart.");
		terminal.println("Type 'CLS' to clear terminal");
		terminal.println("or 'HELLO' to say hello!");
	}
	// Ensure everything is sent
	terminal.flush();
}

// Ustawienie progu temperatury poniżej której załączy się grzałka na winie #1
BLYNK_WRITE(V21)
{
	SetTemp_1 = param.asFloat();
	Wine_Temp_Control(); 	// Uruchomienie funkcji Wine_Temp_Control() aby zadziałało natychmiast
}

// Ustawienie progu temperatury poniżej której załączy się grzałka na winie #2
BLYNK_WRITE(V22)
{
	SetTemp_2 = param.asFloat();
	Wine_Temp_Control(); 	// Uruchomienie funkcji Wine_Temp_Control() aby zadziałało natychmiast
}

// Ustawienie progu temperatury poniżej której załączy się grzałka na winie #3
BLYNK_WRITE(V23)
{
	SetTemp_3 = param.asFloat();
	Wine_Temp_Control(); 	// Uruchomienie funkcji Wine_Temp_Control() aby zadziałało natychmiast
}

// Ustawienie progu temperatury poniżej której załączy się grzałka na winie #4
BLYNK_WRITE(V24)
{
	SetTemp_4 = param.asFloat();
	Wine_Temp_Control(); 	// Uruchomienie funkcji Wine_Temp_Control() aby zadziałało natychmiast
}

// Robi wszystko co powinien
void MainFunction()
{
	ReadTemperaraturesFromDS18B20();	// Odczyt temperatury z czujnika DS18B20
	Read_SHT31_Values();				// Odczyt z czujnika SHT31
	Wyslij_Dane();						// Wysyłanie danych na serwer Blynka
	Wine_Temp_Control();				// Wywołanie funkcji sterującej regulacją temperatury
}

// Definicja strony z wersją firmwaru. Definitions of AutoConnectAux page
static const char Version[] PROGMEM = R"(
{
  "title": "Version",
  "uri": "/page",
  "menu": true,
  "element": [
    {
      "name": "cap",
      "type": "ACText",
      "value": "Version:	1.1.1<br>Date:	30.03.2022"
    }
  ]
}
)";

void setup(void)
{
	// start serial port
	Serial.begin(115200);

	// Pin for relay module set as output
	pinMode( WineRelay_1, OUTPUT );
	digitalWrite( WineRelay_1, HIGH );			// Wyłączenie przekaźnika, ogrzewanie wyłączone
	pinMode( WineRelay_2, OUTPUT );
	digitalWrite( WineRelay_2, HIGH );			// Wyłączenie przekaźnika, ogrzewanie wyłączone
	pinMode( WineRelay_3, OUTPUT );
	digitalWrite( WineRelay_3, HIGH );			// Wyłączenie przekaźnika, ogrzewanie wyłączone
	pinMode( WineRelay_4, OUTPUT );
	digitalWrite( WineRelay_4, HIGH );			// Wyłączenie przekaźnika, ogrzewanie wyłączone

	// Autoconnect
	Config.hostName 		= "Wino_ESP32";							// Sets host name to SotAp identification
	Config.apid 			= "Wino_ESP32";							// SoftAP's SSID.
	Config.psk 				= "12345678";							// Sets password for SoftAP. The length should be from 8 to up to 63.
	Config.homeUri 			= "/_ac";								// Sets home path of Sketch application
	Config.retainPortal 	= true;									// Launch the captive portal on-demand at losing WiFi
	Config.autoReconnect 	= true;									// Automatically will try to reconnect with the past established access point (BSSID) when the current configured SSID in ESP8266/ESP32 could not be connected.
	Config.ota 				= AC_OTA_BUILTIN;						// Specifies to include AutoConnectOTA in the Sketch.
	Portal.load(FPSTR(Version));									// Load AutoConnectAux custom web page
	Config.menuItems = Config.menuItems | AC_MENUITEM_DELETESSID;	// https://hieromon.github.io/AutoConnect/apiconfig.html#menuitems
	Portal.config(Config);											// Don't forget it.
	if (Portal.begin())												// Starts and behaves captive portal
	{	
		Serial.println("WiFi connected: " + WiFi.localIP().toString());
	}

	//WiFi.begin(ssid, pass);
	Blynk.config(auth, server, port);   // for local servernon-blocking, even if no server connection
	//Blynk.config(auth);



	//WiFi.setHostname("ESP32_Rymanowska");
	//WiFi.mode(WIFI_STA);
	//WiFi.begin(ssid, pass);
	Blynk.config(auth, server, port);   		// for local servernon-blocking, even if no server connection
	//Blynk.config(auth);						// For cloud
	blynkCheck(); 								// Piewsze połaczenie z Blynk, nie trzeba czekać 30s po restarcie

	// Ustawienie timera
	timer.setInterval(5000, MainFunction);		// Funkcja zostanie wywołana co 5s

	// Inicjalizaczja czujnika DS18B20
	sensors.begin();
	sensors.setResolution(TEMPERATURE_PRECISION);				// Ustawienie precyzji odczytu temperatury dla wszystkich czujników na raz
	//sensors.setResolution(sensor1, TEMPERATURE_PRECISION);	// Ustawienie precyzji odczytu temperatury
	//sensors.setResolution(sensor2, TEMPERATURE_PRECISION);	// Ustawienie precyzji odczytu temperatury
	//sensors.setResolution(sensor3, TEMPERATURE_PRECISION);	// Ustawienie precyzji odczytu temperatury
	//sensors.setResolution(sensor4, TEMPERATURE_PRECISION);	// Ustawienie precyzji odczytu temperatury

	// Inicjalizaczja czujnika SHT31
	Wire.begin(SDA_Pin, SCI_Pin);  // Można podać zmieniać piny Wire.begin(sdaPin, sciPin); lub nic nie podawać i będzie domyślnie.
	if (! sht31.begin(0x44))	// Set to 0x45 for alternate i2c addr
	{
		Serial.println("Couldn't find SHT31!");
	}
	else
	{
		Serial.println("SHT31 sensor connected.");
	}
}

void loop(void)
{
	timer.run();
	Portal.handleClient();

	if (WiFi.status() == WL_CONNECTED)
	{
				// Here to do when WiFi is connected.
		if (Blynk.connected())
		{
			Blynk.run();
		} 
		else
		{
			Blynk.connect();
		}
		//OTA_Handle();			//Obsługa OTA (Over The Air) wgrywanie nowego kodu przez Wi-Fi
		if(timer.isEnabled( timerIDReset ))
		{
			timer.deleteTimer( timerIDReset );
		}
	}
	else	//Zrestartuje sterownik jeśli brak sieci przez 5min
	{
		if (timer.isEnabled( timerIDReset ) == false)
		{
			timerIDReset = timer.setTimeout( 300000, RestartESP32 ); //300000
		}
	}
}
#include <Arduino.h>

//AutoConnect https://hieromon.github.io/AutoConnect/
#include <ESP8266WiFi.h>		// Replace 'ESP8266WiFi.h' with 'WiFi.h. for ESP32
#include <ESP8266WebServer.h>	// Replace 'ESP8266WebServer.h'with 'WebServer.h' for ESP32
#include <AutoConnect.h>

//WiFiWebServer Server;
ESP8266WebServer	Server;		// Replace 'ESP8266WebServer' with 'WebServer' for ESP32
AutoConnect			Portal(Server);
AutoConnectConfig	Config;

/*
//for OTA
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>				// https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA
bool OTAConfigured = 0;
*/


//#define BLYNK_DEBUG				// Optional, this enables lots of prints
//#define BLYNK_PRINT Serial
//#include <ESP8266WiFi.h> declared for AutoConnect     // ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <BlynkSimpleEsp8266.h>		// https://github.com/blynkkk/blynk-library
#include <SimpleTimer.h>			// https://github.com/jfturcot/SimpleTimer
#include <TimeLib.h>				// https://github.com/PaulStoffregen/Time				
#include <WidgetRTC.h>
#include <math.h>					// Potrzebne tylko do obliczenia log() w funkcji "Thermistor(int RawADC)"
//WidgetBridge	bridge1(V20);		// Initiating Bridge Widget on V20 of Device A
//WidgetTerminal	terminal(V40);	// Attach virtual serial terminal to Virtual Pin V40
WidgetLED		WidgetLamp(V8);		// Inicjacja diody LED dla załączania różowej lampy
WidgetLED		WigdetFan(V9);		// Inicjacja diody LED dla załączania wentylatora
SimpleTimer		Timer;				// Timer do sprawdzania połaczenia z BLYNKiem (co 30s) i uruchamiania MainFunction (co 3s)
WidgetRTC		rtc;				// Inicjacja widgetu zegara czasu rzeczywistego RTC


const int ThermistorPIN = A0;		// ESP8266 Analog Pin ADC0 = A0
float pad 				= 12000;	// balance/pad resistor value, set this to the measured resistance of your pad resistor

int		Tryb_Sterownika	= 0;		// Tryb_Sterownika 0 = AUTO, 1 = ON, 2 = OFF, 3 = MANUAL
float	SetTempActual	= 60;		// Temperatura według której załączany jest wentylator
boolean	GrowLampStatus	= false;	// Domyślnie lampa wyłączona		(false = OFF, true = ON)
boolean	FanStatus		= false;	// Domyślnie wentylator wyłączony	(false = OFF, true = ON)
boolean	LampTempHigh	= false;	// Alarm gdy temperatura lampy przekroczy zbyt wysoka wartość
int		StartHour		= 0;		// Godzina włączenia lampy
int 	StartMinute		= 0;		// Minuta włączenia lampy
int		StopHour		= 0;		// Godzina wyłączenia lampy
int 	StopMinute		= 0;		// Minuta wyłączenia lampy
boolean	DayOfWeek[7]	= {false,false,false,false,false,false,false}; // Dni tygodznia kiedy załączy się lampa
int		dzien			= 1;		// day of the week (1-7), Sunday is day 1
int		godzina			= 0;		// the hour now (0-23)
int		minuta			= 0;		// the minute now (0-59)
float	temp			= 0;		// temperatura radiatora z termistora
 float	TemtHist		= 10;		// Histereza dla temperatury

//STAŁE
//const char	ssid[]		= "Your SSID";							// Not required with AutoConnect
//const char	pass[]		= "Router password";					// Not required with AutoConnect
const char		auth[]	 	= "V8_3uZNcYsMgF3A5a7zn6cOuBR8bA6bR";	// Token Avocado
const char		server[] 	= "192.168.1.204";  					// IP for your Local Server or DNS server addres stecko.duckdns.org
const int		port 		= 8080;									// Port na którym jest serwer Blykn
const int		PlantLED	= D5;		// Pin do włączania światła LED
const int		Fan			= D6;		// Pin do włączania wentylatora
const int		Moisture	= D7;		// Pin do włączania nawilżacza powietrza
const float		TempAlarm	= 70;		// Maksymala dozwolona temperatura lampy

//---------------------------------------------------------------------------------------------------------------------------------------------

//Informacja że połączono z serwerem Blynk, synchronizacja danych
BLYNK_CONNECTED()
{
	Serial.println("Reconnected, syncing with cloud.");
	rtc.begin();
	Blynk.syncAll();
}

//Sprawdza czy połączone z serwerem Blynk
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
/*
//Over-The-Air w skrócie OTA umożliwia przesyłanie plików do urządzeń przez sieć WiFi
void OTA_Handle()
{
	if (OTAConfigured == 1)
	{
		if (WiFi.waitForConnectResult() == WL_CONNECTED) {
			ArduinoOTA.handle();
		}
	}
	else
	{
		if (WiFi.waitForConnectResult() == WL_CONNECTED)
		{
			// Port defaults to 8266
			ArduinoOTA.setPort(8266);

			// Hostname defaults to esp8266-[ChipID]
			ArduinoOTA.setHostname("Avocado");

			// No authentication by default
			//ArduinoOTA.setPassword("admin");

			// Password can be set with it's md5 value as well
			// MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
			// ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

			ArduinoOTA.onStart([]() {
				String type;
				if (ArduinoOTA.getCommand() == U_FLASH) { type = "sketch";}
				else { type = "filesystem";}  // U_SPIFFS
				// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
				Serial.println("Start updating " + type);
			});
			
			ArduinoOTA.onEnd([]() {
				Serial.println("\nEnd");
			});
			
			ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
			Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
			});
			
			ArduinoOTA.onError([](ota_error_t error) {
				Serial.printf("Error[%u]: ", error);
				if (error == OTA_AUTH_ERROR) { Serial.println("Auth Failed");}
				else if (error == OTA_BEGIN_ERROR) { Serial.println("Begin Failed");}
				else if (error == OTA_CONNECT_ERROR) { Serial.println("Connect Failed");}
				else if (error == OTA_RECEIVE_ERROR) { Serial.println("Receive Failed");}
				else if (error == OTA_END_ERROR) { Serial.println("End Failed");}
			});
			
			ArduinoOTA.begin();

			Serial.println("Ready");
			Serial.print("OTA IP address: ");
			Serial.println(WiFi.localIP());

			OTAConfigured = 1;
		}
	}
}
*/
//Ustawienie trybów sterowania lampą LED
void TrybManAuto()
{
	dzien = weekday(now());					// day of the week (1-7), Sunday is day 1
	godzina = hour(now());					// the hour now  (0-23)
	minuta =  minute(now());				// the minute now (0-59)
	if (Tryb_Sterownika == 0 && DayOfWeek[dzien-1] && godzina * 60 + minuta >= StartHour * 60 + StartMinute && godzina * 60 + minuta < StopHour * 60 + StopMinute && StartHour != -1 && StartMinute != -1 && StopHour != -1 && StopMinute != -1)		//Tryb AUTO ON
	{					// Tryb AUTO
		if(GrowLampStatus == false)
		{
			GrowLampStatus = true; 
			WidgetLamp.on();		// Widget lampa włączona
			digitalWrite(PlantLED, LOW);	// Lampa włączona
		}
	}
	else if (Tryb_Sterownika == 0 && (!DayOfWeek[dzien-1] || godzina * 60 + minuta < StartHour * 60 + StartMinute || godzina * 60 + minuta >= StopHour * 60 + StopMinute || StartHour == -1 || StartMinute == -1 || StopHour == -1 || StopMinute == -1))	//Tryb AUTO OFF
	{					//Tryb AUTO
		if(GrowLampStatus == true)
		{
			GrowLampStatus = false; 
			WidgetLamp.off();		// Widget lampa wyłączona
			digitalWrite(PlantLED, HIGH);	// Lampa wyłączona
		}		
	}
	else if (Tryb_Sterownika == 1)		// Tryb ON
	{
		if(GrowLampStatus == false)
		{
			GrowLampStatus = true;
			WidgetLamp.on();		// Widget lampa włączona
			digitalWrite(PlantLED, LOW);	// Lampa włączona
		}
	}
	else if (Tryb_Sterownika == 2)		// Tryb OFF
	{
		if(GrowLampStatus == true)
		{
			GrowLampStatus = false;
			WidgetLamp.off();		// Widget lampa wyłączona
			digitalWrite(PlantLED, HIGH);	// Wentylator wyłączony
		}
	}
}

//Sterowanie wentylatorem w zależności od temperatury z termistora
void Fan_Control()
{
	 if (temp > SetTempActual)
	{
		WigdetFan.on();					//Widget dioda zaświecona
		digitalWrite(Fan, LOW);			//Wentylator załączony
		FanStatus = true;
	}
	else if (temp < SetTempActual - TemtHist)
	{
		WigdetFan.off();				//Widget dioda zgaszona
		digitalWrite(Fan, HIGH);				//Wentylator wyłączony
		FanStatus = false;
	}
}

//Odczyt temperaturu z termistora - zwraca temperaturę w °C
float Thermistor(int RawADC) {
	long Resistance;					//Additional variable  
	float Temp;						//Dual-Purpose variable to save space.

	//Steinhart-Hart Thermistor Equation:
	//* Temperature in Kelvin = 1 / {A + B[ln(R)] + C[ln(R)]3}
	//* where A = 0.001772142418285400, B = 0.000103543286621499 and C = 0.0000007220186952405760


	Resistance = ((1024 * pad / RawADC) - pad);		//Rezystancja zmieżona [Ohm]

	//Serial.print("Resistance = ");
	//Serial.print(Resistance);
	//Serial.print(" Ohm   ");

	Temp = log(Resistance);					//Saving the Log(resistance) so not to calculate  it 4 times later
	Temp = 1 / (0.001772142418285400 + (0.000103543286621499 * Temp) + (0.0000007220186952405760 * Temp * Temp * Temp));
	Temp = Temp - 273.15;					//Convert Kelvin to Celsius 

	//Serial.print("temp = ");
	//Serial.print(Temp,2);
	//Serial.print("°C");
	if (Temp >= TempAlarm)
	{	if (LampTempHigh == false)
		{
			Blynk.notify("Temperatura lampy przekroczyła " + String(Temp) + "°C !");
			LampTempHigh = true;
		}
	}
	return Temp;						//Return the Temperature
}

//funkcja nie używana. Tylko do zbierania charakterystyki termistora 
void TermistorTest(){
	temp = Thermistor(analogRead(ThermistorPIN));		// read ADC and  convert it to Celsius
	Serial.println(" ");
}

//Zwraca siłę sygnału WiFi sieci do której jest podłączony w %. REF: https://www.adriangranados.com/blog/dbm-to-percent-conversion
int WiFi_Strength (long Signal)
{
	return constrain(round((-0.0154*Signal*Signal)-(0.3794*Signal)+98.182), 0, 100);
}

//Wysyłanie danych na serwer Blynka
void Wyslij_Dane()
{
	Blynk.virtualWrite(V0, temp);							// Temperatura [°C]
	Blynk.virtualWrite(V18, SetTempActual);					// Temperatura zadana [°C]

	Blynk.virtualWrite(V25, WiFi_Strength(WiFi.RSSI()));	// Siła sygnału Wi-Fi [%], constrain() limits range of sensor values to between 0 and 100
}

//Sterowanie załączeniem lampy z aplikacji (AUTO, ON, OFF)
BLYNK_WRITE(V11)
{
	switch (param.asInt())
	{
		case 1:						//AUTO
			Tryb_Sterownika = 0;
			break;
		case 2:						//ON
			Tryb_Sterownika = 1;
			break;
		case 3:						//OFF
			Tryb_Sterownika = 2;
			break;
			default:				//Wartość domyślna AUTO
			Tryb_Sterownika = 0;
	}
	TrybManAuto();

}

//Ustawienie progu temperatury poniżej której załączy się wentylator
BLYNK_WRITE(V12)
{
	SetTempActual = param.asFloat();
	Fan_Control();
}

//Obsługa timera Start i Stop (Time Input Widget)
BLYNK_WRITE(V13)
{
	TimeInputParam t(param);
	// Process start time
	if (t.hasStartTime())
	{
		StartHour = t.getStartHour();
		StartMinute = t.getStartMinute();
	}
	else
	{
		StartHour = -1;
		StartMinute = -1;
	}
	

	// Process stop time
	if (t.hasStopTime())
	{
		StopHour = t.getStopHour();
		StopMinute = t.getStopMinute();
	}
	else
	{
		StopHour = -1;
		StopMinute = -1;
	}
	

	// Process weekdays (1. Mon, 2. Tue, 3. Wed, ...) but I need (1. Sat 2. Mon, 3. Tue, 4. Wed, ...)
	if (t.isWeekdaySelected(7))
	{
		DayOfWeek[0] = true;
	}
	else
	{
		DayOfWeek[0] = false;
	}
		
	for (int i = 1; i <= 6; i++)
	{
		if (t.isWeekdaySelected(i))
		{
			DayOfWeek[i] = true;
		}
		else
		{
			DayOfWeek[i] = false;
		}
		
	}
}

//Ustawienie histereza dla temperatury przy której załączy się wentylator
BLYNK_WRITE(V14)
{
	TemtHist = param.asFloat();
	Fan_Control();
}

//Uruchamia po kolei wszystkie niezbędne funcje
void MainFunction()
{	
  	temp = Thermistor(analogRead(ThermistorPIN));		// read ADC and  convert it to Celsius
	if (LampTempHigh == true)
	{	
		if (temp > TempAlarm - 15)
		{
			WidgetLamp.off();				//Widget lampa wyłączona
			digitalWrite(PlantLED, LOW);			//Lampa wyłączona
			GrowLampStatus = false; 
			WigdetFan.on();					//Widget dioda zaświecona
			digitalWrite(Fan, HIGH);			//Wentylator załączony
			FanStatus = true;	
		}
		else
		{
			Blynk.notify("Temperatura lampy spadła do " + String(temp) + "°C !");
			LampTempHigh = false;
		}
	}
	else
	{
		TrybManAuto();
		Fan_Control();
	}

	Wyslij_Dane();
}

//---------------------------------------------------------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);

	// Autoconnect
	Config.apid = "Avocado";			//SoftAP's SSID.
	Config.psk = "12345678";			//Sets password for SoftAP. The length should be from 8 to up to 63.
	Config.homeUri = "/_ac";			// Sets home path of Sketch application
	Config.retainPortal = true;			// Launch the captive portal on-demand at losing WiFi
	Config.autoReconnect = true;		// Automatically will try to reconnect with the past established access point (BSSID) when the current configured SSID in ESP8266/ESP32 could not be connected.
	Config.ota = AC_OTA_BUILTIN;		//Specifies to include AutoConnectOTA in the Sketch.
	//Config.retainPortal = true;		//Continue the portal function even if the captive portal times out. The STA + SoftAP mode of the ESP module continues and accepts the connection request to the AP.
	//Config.immediateStart = true;		//Start the captive portal with AutoConnect::begin.
	Portal.config(Config);				// Don't forget it.
	if (Portal.begin())					// Starts and behaves captive portal
	{	
		Serial.println("WiFi connected: " + WiFi.localIP().toString());
	}

	//WiFi.begin(ssid, pass);
	Blynk.config(auth, server, port);   // for local servernon-blocking, even if no server connection
	//Blynk.config(auth);				//For cloud

	//Inicjalizacja Timerów
	Timer.setInterval(30000, blynkCheck);		//Co 30s zostanie sprawdzony czy jest sieć Wi-Fi i czy połączono z serwerem Blynk
	Timer.setInterval(3000, MainFunction);		//Uruchamia wszystko w pętli co 3s
	//Timer.setInterval(2000, TermistorTest);	//tylko do zbierania charakterystyki termistora

	//Ustawianie pinów
	pinMode(PlantLED, OUTPUT);
	pinMode(Fan, OUTPUT);
	pinMode(Moisture, OUTPUT);

	blynkCheck(); 					//Piewsze połaczenie z Blynk, nie trzeba czekać 30s po restarcie

}

void loop() {
	if (WiFi.status() == WL_CONNECTED)
	{
		// Here to do when WiFi is connected.
		if (Blynk.connected()) Blynk.run();
		Timer.run();
		//OTA_Handle();			//Obsługa OTA (Over The Air) wgrywanie nowego kodu przez Wi-Fi
	}
	else
	{
		//Jeśli nie ma połączenia z siecią to po prostu steruje terrarium na ostatnich pustawieniach
		Fan_Control();
	}
	Portal.handleClient();
}
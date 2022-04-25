#include <Arduino.h>

//Biblioteki potrzebne dla AutoConnect
#include <WiFi.h>				// Replace 'ESP8266WiFi.h' with 'WiFi.h. for ESP32
#include <WebServer.h>			// Replace 'ESP8266WebServer.h'with 'WebServer.h' for ESP32
#include <AutoConnect.h>
WebServer			Server;		// Replace 'ESP8266WebServer' with 'WebServer' for ESP32
AutoConnect			Portal(Server);
AutoConnectConfig	Config;       // Enable autoReconnect supported on v0.9.4
String viewCredential(PageArgument&);
String delCredential(PageArgument&);


//Biblioteka do obsługi wyświetlacza, bardzo szybka
#include <SPI.h>
#include <TFT_eSPI.h>
//#include "Free_Fonts.h" 			// Include the header file attached to this sketch
TFT_eSPI tft = TFT_eSPI();

//BME280 definition
#include <EnvironmentCalculations.h>
#include <Wire.h>

// Include the libraries we need
#include <OneWire.h>
//#include <DallasTemperature.h>
//OneWire oneWire(13); 				// Setup a oneWire instance to communicate with a OneWire device connected to GPIO13
//DallasTemperature DS18B20(&oneWire);	// Pass our oneWire reference to Dallas Temperature sensor 
//DeviceAddress DS18B20_Address = { 0x28, 0xFF, 0x64, 0x18, 0xD0, 0x6F, 0x9D, 0xD5 }; //adres: 28 FF 64 18 D0 6F 9D D5
#include <DS18B20.h>
#define ONE_WIRE_BUS 13				// Setup a oneWire instance to communicate with a OneWire device connected to GPIO13
OneWire oneWire(ONE_WIRE_BUS);
DS18B20 sensor(&oneWire);

//SHT31
#include "Adafruit_SHT31.h"
Adafruit_SHT31 sht31 = Adafruit_SHT31();

//for OTA
//#include <ESPmDNS.h>
//#include <WiFiUdp.h>
//#include <ArduinoOTA.h>
//bool OTAConfigured = 0;

//#define BLYNK_DEBUG				// Optional, this enables lots of prints
/* Comment this out to disable prints and save space */
//#define BLYNK_PRINT Serial
//#include <WiFi.h> //alreadydeclared for AutoConnect
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>		// Replace 'BlynkSimpleEsp8266' with 'BlynkSimpleEsp32.h. for ESP32
#include <SimpleTimer.h>			// https://github.com/jfturcot/SimpleTimer
#include <TimeLib.h>
SimpleTimer Timer;					// Timer do sprawdzania połaczenia z BLYNKiem (co 30s) i uruchamiania MainFunction (co 3s)
int			timerID;				// Przetrzymuje ID Timera, potrzebne dla przycisku i włączania ekranu
int			timerIDReset		= -1;//Przetrzymuje ID Timera https://desire.giesecke.tk/index.php/2018/01/30/change-global-variables-from-isr/

#include <WidgetRTC.h>
WidgetBridge bridge1(V20);			// Initiating Bridge Widget on V20 of Device A
WidgetTerminal terminal(V40);		// Attach virtual serial terminal to Virtual Pin V40
WidgetLED	LED_CO(V8);				// Inicjacja diody LED dla załączania CO
WidgetRTC	rtc;					// Inicjacja widgetu zegara czasu rzeczywistego RTC

int			Tryb_Sterownika		= 0;		// Tryb_Sterownika 0 = AUTO, 1 = ON, 2 = OFF, 3 = MANUAL
float		SetTempManual		= 21;		// Temperatura nastawiana manualnie z aplikacji Blynka 
float		SetTempActual		= 18.5;		// Temperatura według której sterowana jest temperatura (auto lub manual)
float		SetTempSchedule[7][24]	= {
//00:00 01:00 02:00 03:00 04:00 05:00 06:00 07:00 08:00 09:00 10:00 11:00 12:00 13:00 14:00 15:00 16:00 17:00 18:00 19:00 20:00 21:00 22:00 23:00
  {18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 18.7 },  // Niedziela
  {18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 18.7 },  // Poniedziałek
  {18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 18.7 },  // Wtorek
  {18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 18.7 },  // Środa
  {18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 18.7 },  // Czwartek
  {18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 20.6, 18.7 },  // Piątek
  {18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 18.7, 20.6, 18.7 }   // Sobota
}; //Ustawienia temperatury dla poszczególnych dni/godzin, temperatura musi być pomiędzy 14 a 26 stopni Celsjusza z dokładnością do 1 miejsca po przecinku
int			OLED_ON				= 1;		// Przyjmuje wartość '1' dla OLED włączony i '0' dla OLED wyłączony
boolean		Podlane				= true;		// Przyjmuje wartość true gdy podlane czyli wilgotność >80% i false gdy sucho wilgotność <60%
boolean		SoilNotification70	= false;	// Przyjmuje wartość true gdy wysłano notyfikacje o wilgotności 70%, true po podlaniu
boolean		SoilNotification60	= false;	// Przyjmuje wartość true gdy wysłano notyfikacje o wilgotności 60%, true po podlaniu
boolean		SoilNotification50	= false;	// Przyjmuje wartość true gdy wysłano notyfikacje o wilgotności 50%, true po podlaniu
boolean		RestartESP			= false;	// Gdy true i w terminalu YES wykona restart
float		RH					= 0;		// Soil Relative Humidity in % 
long		CZAS_START_MANUAL	= 0;		// Ustawienie czasu przejścia sterowania w trym MANUAL (wartość w sekundach)
long		CZAS_START_AUTO		= 0;		// Ustawienie czasu przejścia sterowania w trym AUTO (wartość w sekundach)
int			dzien				= 0;		// day of the week (1-7), Sunday is day 1
int			godz				= 1;		// the hour now (0-23)
float		dewPoint(NAN), absHum(NAN), heatIndex(NAN);	// Zmienne dla danych z czujnika BME280
float		Temp_SHT31t			= 1;		// Temperatura z czujnika SHT31 [°C]
float		Hum_SHT31t			= 2;		// Wilgotność z czujnika SHT31 [%]
float		Temp_DS18B20		= 3;		// Temperatura z czujnika DS18B20 [°C]

bool		backlight			= HIGH;
#define 	BUTTON				  16		// pin dla przycisku
#define		backlightPin		  2			// 2 corresponds to GPIO2
#define		HeatCO				  12		// Pin do włączania CO w sterowniku w łazience (D6 = GPIO12)
boolean		isScreenON			= true;		// TRUE jeśli ekran świeci FALSE jeśli nie świeci

//STAŁE
//const char	ssid[]			= "Your SSID";							// Not required with AutoConnect
//const char	pass[]			= "Router password";					// Not required with AutoConnect
const char		auth[]			= "34E3Pay9KeL3M6nmeageKsxROS5lho-T";	// Token Pokój Rymanowska
const char		server[] 		= "192.168.1.204";  					// IP for your Local Server or DNS server addres stecko.duckdns.org
const int		port 			= 8080;									// Port na którym jest serwer Blykn
const int		MinTemp			= 14;		// Najniższa możliwa temperatura do ustawienia
const int		MaxTemp			= 26;		// Najwyższa możliwa temperatura do ustawienia
const float		TemtHist		= 0.2;		// histereza dla temperatury
const float		HumidHist		= 5;		// histereza dla wilgotności
const String	hostname 		= "PokojRymanowska_ESP32";
//---------------------------------------------------------------------------------------------------------------------------------------------


//Draw a progress bar - increasing percentage only
void drawProgressBar(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t percent, uint16_t frameColor, uint16_t barColor) {
  if (percent == 0) {
    tft.fillRoundRect(x, y, w, h, 3, TFT_BLACK);
  }
  uint8_t margin = 2;
  uint16_t barHeight = h - 2 * margin;
  uint16_t barWidth  = w - 2 * margin;
  tft.drawRoundRect(x, y, w, h, 3, frameColor);
  tft.fillRect(x + margin, y + margin, barWidth * percent / 100.0, barHeight, barColor);
}

// Soft restart sterownika
void RestartESP32()
{
	ESP.restart(); 	//Restartuje sterownik
}

//Informacja że połączono z serwerem Blynk, synchronizacja danych
BLYNK_CONNECTED()
{
	Serial.println("Reconnected, syncing with cloud.");
	bridge1.setAuthToken("GkXIcT6IWvb0w6RTJyVWsQUe5u5bipe3"); // Token of the hardware B (Łazienka)
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

/*//Over-The-Air w skrócie OTA umożliwia przesyłanie plików do urządzeń przez sieć WiFi
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
			// Port defaults to 3232
			ArduinoOTA.setPort(3232);

			// Hostname defaults to esp8266-[ChipID]
			ArduinoOTA.setHostname("Pokoj_Rym_ESP32");

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
			//digitalWrite(2, HIGH);	//Ustawienie wartośći HIGH = podświetlanie włączone
			Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
			//if (progress < 1) tft.fillScreen(TFT_BLACK);
			//tft.setCursor(60, 60);
			//tft.setTextColor(TFT_WHITE, TFT_BLACK);
			//tft.setTextWrap(true);
			//tft.println("OTA upload...");
			//drawProgressBar(10, 130, 20, 22, (progress / (total / 100)), 0x06DC, 0x3FE0);
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

//Ustawienie trybów sterowania i temperatury do załączenia pieca CO
void TrybManAuto()
{
	//Start Manual za pomocą Time Input
	if (CZAS_START_MANUAL > 0 && hour(now()) * 60 + minute(now()) == CZAS_START_MANUAL / 60)	//Tryb_Sterownika 0 = AUTO, 1 = ON, 2 = OFF, 3 = MANUAL
	{
		CZAS_START_MANUAL = 0;		//Zerowanie czasu uruchomienia tryby MANUALNEGO, czyli wyłączenie załączanie czasowego
		Blynk.virtualWrite(V11, 4);	//Ustawienie widgetu BLYNK w tryb MAN
		Tryb_Sterownika = 3;		//Tryb_Sterownika 0 = AUTO, 1 = ON, 2 = OFF, 3 = MANUAL
		Blynk.setProperty(V12,"color","#ff9b00");
		//reset Time Input widget in app
		if (CZAS_START_AUTO == 0)
		{
			CZAS_START_AUTO = - 1;
		}
		Blynk.virtualWrite(V13, -1, CZAS_START_AUTO, "Europe/Warsaw");		//Kasowanie StartManual
	}
	// Start Auto za pomocą Time Input
	if (CZAS_START_AUTO > 0 && hour(now()) * 60 + minute(now()) == CZAS_START_AUTO / 60)		//Tryb_Sterownika 0 = AUTO, 1 = ON, 2 = OFF, 3 = MANUAL
	{
		CZAS_START_AUTO = 0;		//Zerowanie czasu uruchomienia tryby AUTOMATYCZNEGO, czyli wyłączenie załączanie czasowego
		Blynk.virtualWrite(V11, 1);	//Ustawienie widgetu BLYNK w tryb AUTO
		Tryb_Sterownika = 0;		//Tryb_Sterownika 0 = AUTO, 1 = ON, 2 = OFF, 3 = MANUAL
		Blynk.setProperty(V12,"color","#403b34");
		//reset Time Input widget in app
		if (CZAS_START_MANUAL == 0)
		{
			CZAS_START_MANUAL = - 1;
		}
		Blynk.virtualWrite(V13, CZAS_START_MANUAL, -1, "Europe/Warsaw");	//Kasowanie StartAuto
	}

	dzien = weekday(now()) - 1;			//day of the week (1-7), Sunday is day 1
	godz = hour(now());					//the hour now  (0-23)
	//Ustawienie temperatury sterowania
	if (Tryb_Sterownika == 0)		//Tryb AUTO,  temperatura w zmiennej SetTempActual ustawiana tylko jeśli różna od zadanej w SetTempSchedule
	{
		if(SetTempActual != SetTempSchedule[dzien][godz])
		{
			SetTempActual = SetTempSchedule[dzien][godz]; 
		}
	}
	else if (Tryb_Sterownika == 1)		//Tryb ON,  temperatura w zmiennej SetTempActual ustawiana tylko jeśli różna od zadanej w SetTempSchedule
	{
		if(SetTempActual != Temp_SHT31t)
		{
			SetTempActual = Temp_SHT31t;
		}
	}
	else if (Tryb_Sterownika == 2)		//Tryb OFF,  temperatura w zmiennej SetTempActual ustawiana tylko jeśli różna od 0 (zera)
	{
		if(SetTempActual != 0)
		{
			SetTempActual = 0;
		}
	}
	else if (Tryb_Sterownika == 3)		//Tryb MAN,  temperatura w zmiennej SetTempActual ustawiana tylko jeśli różna od zadanej w SetTempManual
	{
		if (SetTempActual != SetTempManual)
		{
			SetTempActual = SetTempManual;
		}
	}
}

//Odczyt wskazań z czujnika SHT31
void Read_SHT31_Values()
{
	Temp_SHT31t	= sht31.readTemperature() -1.5;		//Korekta dla temperatury dla czujnika SHT31
	Hum_SHT31t	= sht31.readHumidity() + 0;		//Korekta poziomu wilgotności dla czujnika SHT31

	EnvironmentCalculations::TempUnit     envTempUnit =  EnvironmentCalculations::TempUnit_Celsius;
	//Dane obliczane na podstawie danych z czujnika
	dewPoint	= EnvironmentCalculations::DewPoint(Temp_SHT31t, Hum_SHT31t, envTempUnit);
	absHum		= EnvironmentCalculations::AbsoluteHumidity(Temp_SHT31t, Hum_SHT31t, envTempUnit);
	heatIndex	= EnvironmentCalculations::HeatIndex(Temp_SHT31t, Hum_SHT31t, envTempUnit);
}

//Odczyt wskazań z czujnika BME280
void Read_DS18B20_Values()
{
	//DS18B20.requestTemperatures(); 				// Wystłanie prośby o odczyt temperatury, bedzie gotowa na potem
	//Temp_DS18B20	= DS18B20.getTempC(DS18B20_Address) - 0.8;	// Odczyt temperatury

	if(sensor.isConversionComplete())		// wait until sensor is ready
	{
		Serial.print("Temp: ");
		Serial.println(sensor.getTempC());
		Temp_DS18B20	= sensor.getTempC() - 0.94;	// Odczyt temperatury
		sensor.requestTemperatures();
	}

}

//Działa jak map() ale zwraca liczby rzeczywiste a nie tylko całkowite
double mapf(double val, double in_min, double in_max, double out_min, double out_max)
{
    return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//Odczyt z czujnika wilgotności gleby i konwersja do wartości 0 - 100%
float ReadSoilMoisture()
{
	int i;
	float sval = 0;
	int shArray[6];
	int maxSH = 0;
	int minSH = 4095;
	for (i = 0; i < 5; i++)
	{   //Uśrednianie wartości z czujnika analogowego (5 wartości pomijając ekstrema)
	delay(5);
	shArray[i] = analogRead(34);
	if ( shArray[i] > maxSH ) maxSH = shArray[i];
	if ( shArray[i] < minSH ) minSH = shArray[i];
	sval = sval + shArray[i];
	}
	sval = sval - maxSH - minSH;
	sval = sval / 3;
	sval = mapf(sval, 3550, 2000, 0, 100);  //Convert to Relative Humidity in % (818 -> sensor in air, 427 -> sensor in water)
	//sval = constrain(sval, 0, 100);   //Limits range of sensor values to between 10 and 150
	//informacja o podlaniu
	if (sval > 80 && Podlane == false)
	{
		Podlane = true;			//Ustawia wartość na true czyli podlano kwiatka
		SoilNotification50 = false;	//Ustawia wartość na true czyli zeruje ilość powiadomień po podlaniu kwiatka
		SoilNotification60 = false;	//Ustawia wartość na true czyli zeruje ilość powiadomień po podlaniu kwiatka
		SoilNotification70 = false;	//Ustawia wartość na true czyli zeruje ilość powiadomień po podlaniu kwiatka
		Blynk.notify("Kwiatek został podlany :)");
	}
	else if (sval < 70 && Podlane == true)
	{
		Podlane = false;		//Ustawia wartość na false czyli kwiatek ma sucho
	}
	//Powiadomienia na smartfona
	if (sval < 50 && Podlane == false && SoilNotification50 == false)
	{
		Blynk.notify("Podlej tego kwiatka bo uschnie! Wilgotność spadła już do 50%.");
		SoilNotification50 = true;	//Ustawia wartość na true czyli powiadomienie przy wilgotności 40% poszło
	}
	else if (sval < 60 && Podlane == false  && SoilNotification60 == false)
	{
		Blynk.notify("Wilgotność w kwiatku spadła poniżej 60%. Teraz już trzeba podlać!");
		SoilNotification60 = true;	//Ustawia wartość na true czyli powiadomienie przy wilgotności 50% poszło
	}
	else if (sval < 70 && Podlane == false && SoilNotification70 == false)
	{
		Blynk.notify("Wilgotność w kwiatku spadła poniżej 70%. Może trzeba podlać?");
		SoilNotification70 = true;	//Ustawia wartość na true czyli powiadomienie przy wilgotności 60% poszło
	}

return sval;
}

//Sterowanie piecem w zależności od temperatury
void Room_Temp_Control()
{
	if (Tryb_Sterownika == 1)
	{
		bridge1.digitalWrite(HeatCO, 0);	//Wysłanie sygnału do włączenia pieca
		bridge1.virtualWrite(V18, true); 	//Wysłanie informacji do sterownika w łazience że piec grzeje
		LED_CO.on();				//Piec CO grzeje
		digitalWrite(LED_BUILTIN, LOW);		//Niebieska dioda WEMOSA gaśnie
	}
	else if (Temp_SHT31t < SetTempActual - TemtHist)
	{
		bridge1.digitalWrite(HeatCO, 0);	//Wysłanie sygnału do włączenia pieca
		LED_CO.on();				//Piec CO grzeje
		bridge1.virtualWrite(V18, true); 	//Wysłanie informacji do sterownika w łazience że piec grzeje
		digitalWrite(LED_BUILTIN, LOW);		//Niebieska dioda WEMOSA gaśnie
	}
	else if (Temp_SHT31t > SetTempActual + TemtHist)
	{
		bridge1.digitalWrite(HeatCO, 1023);	//Wysłanie sygnału do wyłączenia pieca (1023 bo piny obsługują PWM i nadanie "1" nie działa)
		LED_CO.off();				//Piec CO nie grzeje
		bridge1.virtualWrite(V18, false); 	//Wysłanie informacji do sterownika w łazience że piec nie grzeje
		digitalWrite(LED_BUILTIN, HIGH);	//Niebieska dioda WEMOSA świeci
	}
}

//Wyłączenie podświetlania sedesu
void ScreenOFF()
{
	backlight = LOW;
	delay(10);
	digitalWrite(2, backlight);			//Ustawienie wartośći HIGH = podświetlanie włączone
	isScreenON = false;
	tft.fillScreen(TFT_BLACK);
}

//Wyświetlanie na ekranie LED 1,3""
void OLED_Display()
{
	if (isScreenON)
	{
		//Dla nadpisywania poprzedniej wartości https://github.com/Bodmer/TFT_eSPI/blob/master/examples/480%20x%20320/TFT_Padding_demo/TFT_Padding_demo.ino
		byte font = 2;
		int padding = tft.textWidth("999.9999", font); // get the width of the text in pixels
		tft.setTextPadding(padding);
		//tft.fillScreen(TFT_BLACK);
		//tft.setFreeFont(&FreeMonoBold12pt7b);
		tft.setTextSize(1);
		//Temperature
		tft.setCursor(0, 15);
		tft.setTextColor(TFT_ORANGE, TFT_BLACK);
		tft.println(" Set:                 " + String(SetTempActual) + " `C");
		tft.setTextColor(TFT_RED, TFT_BLACK);
		tft.println(" Actual:           " + String(Temp_SHT31t) +" `C");
		//Wilgotność powietrza
		tft.setTextColor(TFT_BLUE, TFT_BLACK);
		tft.println(" Hum:              " + String(Hum_SHT31t) + " %");
		//Wilgotność gleby
		tft.setTextColor(TFT_GREEN, TFT_BLACK);
		tft.println(" SMoist:         " + String(RH) + " %");
		//Temperatura DS18B20
		tft.setTextColor(TFT_CYAN, TFT_BLACK);
		tft.println(" DS18B20:   " + String(Temp_DS18B20) + " `C");



		//tft.setTextPadding(padding);
		//tft.drawFloat(Temp_SHT31t, 20, 10, 150, 4);
	}
	else
	{
		ScreenOFF();
	}
	
}

//Zwraca siłę sygnału WiFi sieci do której jest podłączony w %. REF: https://www.adriangranados.com/blog/dbm-to-percent-conversion
int WiFi_Strength (long Signal)
{
	return constrain(round((-0.0154*Signal*Signal)-(0.3794*Signal)+98.182), 0, 100);
}

//signal strength levels https://www.netspotapp.com/what-is-rssi-level.html
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

//Wysyłanie danych na serwer Blynka
void Wyslij_Dane()
{

	Blynk.virtualWrite(V0, Temp_SHT31t);			// Temperatura z czujnika SHT31 [°C]
	Blynk.virtualWrite(V1, Hum_SHT31t);				// Wilgotność z czujnika SHT31 [%]
	Blynk.virtualWrite(V2, Temp_DS18B20);			// Temperatura z czujnika DS18B20 [°C]
	Blynk.virtualWrite(V3, dewPoint);				// Temperatura punktu rosy [°C]
	Blynk.virtualWrite(V4, absHum);					// Wilgotność bezwzględna [g/m³]
	Blynk.virtualWrite(V5, heatIndex);				// Temperatura odczuwalna [°C] 
	Blynk.virtualWrite(V6, RH);						// Wilgotność gleby [%]  
	Blynk.virtualWrite(V18, SetTempActual);			// Temperatura zadana [°C]
	Blynk.virtualWrite(V25, WiFi_Strength(WiFi.RSSI()));	// Siła sygnału Wi-Fi [%], constrain() limits range of sensor values to between 0 and 100
	bridge1.virtualWrite(V21, Hum_SHT31t);			// Wilgotność w pokoju wysyłana do sterownika w łazience [%]
}

//Obsługa przerwań wywoływanych przez cprzycisk
//ICACHE_RAM_ATTR void handleInterrupt()
IRAM_ATTR void handleInterrupt() 
{
	
	if ( isScreenON && Timer.isEnabled(timerID))
	{
		Timer.restartTimer(timerID);				//Wydłuża iluminacje sedesu o kolejne 30s
	}
	else if (isScreenON == false)					//JEśli ekran jest wyłączony
	{
		timerID = Timer.setTimeout(50000, ScreenOFF);//Wyłączy iluminacje sedesu za 50s
		backlight = HIGH;
		digitalWrite(2, backlight);			//Ustawienie wartośći HIGH = podświetlanie włączone
		isScreenON = true;
	}

}

//Wyłącza możliwość restartu ESP (timer ustawiony na 30s)
void RestartCounter()
{
	RestartESP = false;	// Gdy true i w terminalu YES wykona restart
}

//Obsługa terminala
BLYNK_WRITE(V40)
{
	String TerminalCommand = param.asStr();
	TerminalCommand.toLowerCase();

	if (String("ports") == TerminalCommand)
	{
		terminal.clear();
		terminal.println("PORT    DESCRIPTION        UNIT");
		terminal.println("V0   -> SHT31 temp         °C");
		terminal.println("V1   -> SHT31 hum          %");
		terminal.println("V2   -> DS18B21 temp       °C");
		terminal.println("V3   -> DewPoint           °C");
		  terminal.print("V4   -> Abs Humidity       g/m");
		terminal.println("\xc2\xb3");	// \xc2\xb3 -->  ^3 potęga 3 https://www.utf8-chartable.de/unicode-utf8-table.pl?start=128&number=128&utf8=string-literal&unicodeinhtml=hex
		terminal.println("V5   -> Heat Index         °C");
		terminal.println("V6   -> Wilgotność gleby   %");
		terminal.println("V10  <- OLED_ON            1/0");
		terminal.println("V11  <- Tryb_Sterownika    1,2,3,4");
		terminal.println("V12  <- SetTempManual      °C");
		terminal.println("V13  <- BLYNK Timer        sec");
		terminal.println("V25  -> WiFi Signal        %");
		terminal.println("V40 <-> Terminal           String");
	}
	else if (String("values") == TerminalCommand)
	{
		terminal.clear();
	      terminal.println("PORT   DATA         VALUE ");
		terminal.print("V0   SHT31 temp   = ");
		terminal.print(Temp_SHT31t);
		terminal.println(" °C");
		terminal.print("V1   SHT31 hum    = ");
		terminal.print(Hum_SHT31t);
		terminal.println(" %");
		terminal.print("V2   DS18B21 temp = ");
		terminal.print(Temp_DS18B20);
		terminal.println(" °C");
		terminal.print("V3   DewPoint     = ");
		terminal.print(dewPoint);
		terminal.println(" °C");
		terminal.print("V4   Abs Humidity = ");
		terminal.print(absHum);
		terminal.print(" g/m");
		terminal.println("\xc2\xb3");	// \xc2\xb3 -->  ^3 potęga 3 https://www.utf8-chartable.de/unicode-utf8-table.pl?start=128&number=128&utf8=string-literal&unicodeinhtml=hex
		terminal.print("V5   Heat Index   = ");
		terminal.print(heatIndex);
		terminal.println(" °C");
		terminal.print("V10  OLED_ON      = ");
		terminal.println(OLED_ON);
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
		terminal.print("MinTemp    =  ");
		terminal.print(MinTemp);
		terminal.println("°C");		
		terminal.print("MaxTemp    =  ");
		terminal.print(MaxTemp);
		terminal.println("°C");			
		terminal.print("TemtHist   = ±");
		terminal.print(TemtHist);
		terminal.println("°C");
		terminal.print("HumidHist  = ±");
		terminal.print(HumidHist);
		terminal.println("%");
	}
	else if (String("restart") == TerminalCommand)
	{
		terminal.clear();
		terminal.println("Are you sure you want to restart?");
		terminal.println("Type YES if you are sure to restart...");
		RestartESP = true;	//Gdy true i w terminalu YES wykona restart
		Timer.setTimeout(30000, RestartCounter);
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
	else if (String("autotemp") == TerminalCommand)
	{
		terminal.clear();
		terminal.println("H   |Ni  |Pn   |Wt   |Śr   |Czw  |Pi   |So ");
		int i;
		int j;

		for (i = 0; i < 24; i++)
		{		//wyświeltanie zawartości tabeli SetTempSchedule[7][24]
			if (i<10) terminal.print("0");
			terminal.print(i);
			terminal.print("  ");
			for (j = 0; j < 6; j++)
			{
				terminal.print(String(SetTempSchedule[j][i],1));
				terminal.print(" |");
			}
			terminal.print(String(SetTempSchedule[6][i],1));
			terminal.println("°C");
		}
	}
	else
	{
		terminal.clear();
		terminal.println("Type 'PORTS' to show list");
		terminal.println("Type 'VALUES' to show sensor data");
		terminal.println("Type 'CONSTANT' show constant values");
		terminal.println("Type 'RESTART' to restart.");
		terminal.println("Type 'CLS' to clear terminal");
		terminal.println("Type 'AUTOTEMP' to show Temp Schedule");
		terminal.println("or 'HELLO' to say hello!");
	}
	// Ensure everything is sent
	terminal.flush();
}

//Włączanie i wyłączanie wyświetlacza
BLYNK_WRITE(V10)
{
	OLED_ON = param.asInt(); 
}

//Sterowanie ogrzewaniem z aplikacji (AUTO, ON, OFF, MAN)
BLYNK_WRITE(V11)
{
	switch (param.asInt())
	{
		case 1:				// AUTO
			Tryb_Sterownika = 0;
			Blynk.setProperty(V12,"color","#403b34");
			break;
		case 2:				// ON
			Tryb_Sterownika = 1;
			Blynk.setProperty(V12,"color","#403b34");
			break;
		case 3:				// OFF
			Tryb_Sterownika = 2;
			Blynk.setProperty(V12,"color","#403b34");
			break;
		case 4:				// MAN
			Tryb_Sterownika = 3;
			Blynk.setProperty(V12,"color","#ff9b00");
			//Blynk.setProperty(V12,"label","SetT");
			break;
		default:		// Wartość domyślna AUTO
			Tryb_Sterownika = 0;
			Blynk.setProperty(V12,"color","#403b34");
	}
	TrybManAuto();			//Uruchomienie funkcji TrybManAuto() aby zadziałało natychmiast
	Room_Temp_Control(); 	//Uruchomienie funkcji Room_Temp_Control() aby zadziałało natychmiast

}

//Ustawienie progu temperatury poniżej której załączy się CO (plus próg)
BLYNK_WRITE(V12)
{
	SetTempManual = param.asFloat();
	Room_Temp_Control(); 	// Uruchomienie funkcji Room_Temp_Control() aby zadziałało natychmiast
}

//Obsługa timera Start Manual i Start Auto (Time Input Widget)
BLYNK_WRITE(V13)
{
	CZAS_START_MANUAL = param[0].asLong();		// Ustawienie czasu przejścia sterowania w trym MANUAL (wartość w sekundach)
	CZAS_START_AUTO = param[1].asLong();		// Ustawienie czasu przejścia sterowania w trym AUTO (wartość w sekundach)
}

//Robi wszystko co powinien
void MainFunction()
{
	Read_SHT31_Values();		// Odczyt danych z czujnika SHT31
	RH = ReadSoilMoisture();	// Odczyt z czujnika wilgotności gleby i konwersja do wartości 0 - 100%
	Read_DS18B20_Values();		// Odczyt danych z czujnika DS18B20
	TrybManAuto();				// Ustawienie trybów sterowania i temperatury do załączenia pieca CO
	Room_Temp_Control();		// kontrola temperatury na podstawie odczytów z BME280
	OLED_Display();				// Wyświetlanie na ekranie OLED 0.96"
	Wyslij_Dane();				// Wysyła dane do serwera Blynk
	if (Timer.isEnabled(timerID) == false && backlight == HIGH)
	{
		ScreenOFF();			// Wyłączenie podświetlania sedesu
	}
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
      "value": "Version: 1.1.1<br>Date: 30.03.2022"
    }
  ]
}
)";

/***********************************************************************************************/

void setup()
{
	Serial.begin(115200);
	tft.init();				// initialize a ST7789 chip, 240x240 pixels
	tft.fillScreen(TFT_BLACK);
	tft.setTextFont(4);
	tft.setCursor(0, 0);
	//tft.setTextColor(WHITE);
	tft.setTextColor(TFT_WHITE, TFT_BLACK);
	tft.setTextWrap(true);
	//tft.setRotation(0);
	tft.println("ST7789 Initialized");
	Serial.println("ST7789 Initialized");
	delay(1500);

	//Deklaracja Pinów
	pinMode(2, OUTPUT);							// Zasilanie ekranu - podświetlanie
	digitalWrite(2, HIGH);						// Ustawienie wartośći HIGH = podświetlanie włączone
	pinMode(BUTTON, INPUT_PULLUP);

	//Ustawienia ADC
	//analogSetSamples(1);						// Set number of samples in the range, default is 1, it has an effect on sensitivity has been multiplied
	analogSetClockDiv(255);

	attachInterrupt(digitalPinToInterrupt(BUTTON), handleInterrupt, RISING);	// Obsługa przerwań dla czujnika ruchu
	timerID = Timer.setTimeout(50000, ScreenOFF);					// Wyłączy iluminacje sedesu za 500s
	
	// Autoconnect
	Config.hostName 		= "PokojRymanowska_ESP32";		// Sets host name to SotAp identification
	Config.apid 			= "PokojRymanowska_ESP32";		// SoftAP's SSID.
	Config.psk 				= "12345678";					// Sets password for SoftAP. The length should be from 8 to up to 63.
	Config.homeUri 			= "/_ac";						// Sets home path of Sketch application
	Config.retainPortal 	= true;							// Launch the captive portal on-demand at losing WiFi
	Config.autoReconnect 	= true;							// Automatically will try to reconnect with the past established access point (BSSID) when the current configured SSID in ESP8266/ESP32 could not be connected.
	Config.ota 				= AC_OTA_BUILTIN;				// Specifies to include AutoConnectOTA in the Sketch.
	Portal.load(FPSTR(Version));							// Load AutoConnectAux custom web page
	Config.menuItems = Config.menuItems | AC_MENUITEM_DELETESSID;	// https://hieromon.github.io/AutoConnect/apiconfig.html#menuitems
	Portal.config(Config);									// Don't forget it.


	// Here to do when WiFi is not connected.
	tft.fillScreen(TFT_BLACK);
	tft.setCursor(0, 0);
	tft.setTextSize(1);
	tft.setTextColor(TFT_RED, TFT_BLACK);
	tft.println(" No WiFi connection !");
	tft.println(" Use captive portal:");
	tft.setTextColor(TFT_YELLOW, TFT_BLACK);
	tft.println(" ");
	tft.print(" APID:  ");
	tft.setTextColor(TFT_WHITE, TFT_BLACK);
	tft.println(AUTOCONNECT_APID);
	tft.setTextColor(TFT_YELLOW, TFT_BLACK);
	tft.print(" Pass:  ");
	tft.setTextColor(TFT_WHITE, TFT_BLACK);
	tft.println(AUTOCONNECT_PSK);
	
	if (Portal.begin())
	{	
		tft.fillScreen(TFT_BLACK);
		tft.setCursor(0, 0);
		tft.println("WiFi connected:");
		tft.println(" ");
		tft.println(WiFi.localIP().toString());
		Serial.println("WiFi connected: " + WiFi.localIP().toString());
		delay(3000);
	}
	//WiFi.setHostname("ESP32_Rymanowska");
	//WiFi.mode(WIFI_STA);
	//WiFi.begin(ssid, pass);
	Blynk.config(auth, server, port);   // for local servernon-blocking, even if no server connection
	//Blynk.config(auth);				// For cloud


	//Inicjalizacja Timerów
	Timer.setInterval(30000, blynkCheck);		// Co 30s zostanie sprawdzony czy jest sieć Wi-Fi i czy połączono z serwerem Blynk
	Timer.setInterval(3000, MainFunction);		// Uruchamia wszystko w pętli co 3s

	//Inicjalizaczja czujnika SHT31
	Wire.begin(33,32);  //Można podać zmieniać piny Wire.begin(sdaPin, sciPin); lub nic nie podawać i będzie domyślnie.
	//sht31.heater(false);				// Wyłącza podgrzewanie sensora nie działa
	if (! sht31.begin(0x44))
	{	// Set to 0x45 for alternate i2c addr
		Serial.println("Couldn't find SHT31");
		tft.fillScreen(TFT_BLACK);  // Czyszczenie ekranu
		tft.setTextColor(TFT_RED);
		tft.setTextWrap(true);
		//tft.setFreeFont(&FreeSans9pt7b);
		tft.setCursor(3, 20);
		tft.print("Couldn't find valid SHT31 sensor, check wiring!");
		while (1) delay(1);
	}


	//Inicjalizaczja czujnika DS18B20
	Serial.println(__FILE__);
	Serial.print("DS18B20 Library version: ");
	Serial.println(DS18B20_LIB_VERSION);
	sensor.begin();
	sensor.requestTemperatures();
	

	//Ustawianie pinów
	pinMode(LED_BUILTIN, OUTPUT);			//Będzie mrugał diodą

	//inicjowanie wyświetlacza
	tft.fillScreen(TFT_BLACK);  	// Czyszczenie ekranu
	tft.setCursor(0, 0);
	tft.setTextColor(TFT_WHITE, TFT_BLACK);
	tft.setTextWrap(false);
	tft.setTextSize(2);
	tft.println(" ESP32:");
	tft.setTextSize(1);
	tft.setTextColor(TFT_WHITE, TFT_BLACK);
	tft.println("");
	delay(2000);
	tft.fillScreen(TFT_BLACK);  	// Czyszczenie ekranu

	blynkCheck(); 					// Piewsze połaczenie z Blynk, nie trzeba czekać 30s po restarcie
}

void loop()
{
	Timer.run();
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
		if(Timer.isEnabled( timerIDReset ))
		{
			Timer.deleteTimer( timerIDReset );
		}
	}
	else	//Zrestartuje sterownik jeśli brak sieci przez 5min
	{
		if (Timer.isEnabled( timerIDReset ) == false)
		{
			timerIDReset = Timer.setTimeout( 300000, RestartESP32 ); //300000
		}
		// Here to do when WiFi is not connected.
		delay(10);
		digitalWrite(2, HIGH);			//Ustawienie wartośći HIGH = podświetlanie włączone
		isScreenON = true;
		tft.fillScreen(TFT_BLACK);
		tft.setTextFont(4);
		tft.setCursor(0, 0);
		tft.setTextSize(1);
		tft.setTextColor(TFT_RED, TFT_BLACK);
		tft.println(" No WiFi connection!");
		delay(1500);
	}

}
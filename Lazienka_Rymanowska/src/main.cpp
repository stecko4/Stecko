#include <Arduino.h>

//AutoConnect https://hieromon.github.io/AutoConnect/
#include <ESP8266WiFi.h>		// Replace 'ESP8266WiFi.h' with 'WiFi.h. for ESP32
#include <ESP8266WebServer.h>	// Replace 'ESP8266WebServer.h'with 'WebServer.h' for ESP32
#include <AutoConnect.h>

//WiFiWebServer Server;
ESP8266WebServer	Server;		// Replace 'ESP8266WebServer' with 'WebServer' for ESP32
AutoConnect			Portal(Server);
AutoConnectConfig	Config;

//BME280 definition
#include <EnvironmentCalculations.h>			//https://github.com/finitespace/BME280/blob/master/src/EnvironmentCalculations.h
#include <BME280I2C.h>
#include <SPI.h>
#include <Wire.h>
BME280I2C::Settings settings(
	BME280::OSR_X1,
	BME280::OSR_X1,
	BME280::OSR_X1,
	BME280::Mode_Forced,
	BME280::StandbyTime_125ms,
	BME280::Filter_Off,
	BME280::SpiEnable_False,
	BME280I2C::I2CAddr_0x76 // I2C address. I2C specific.
	);
BME280I2C bme(settings);

//for OTA
//#include <ESP8266mDNS.h>
//#include <WiFiUdp.h>
//#include <ArduinoOTA.h>
//bool OTAConfigured = 0;

//#define BLYNK_DEBUG					// Optional, this enables lots of prints
//#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>			// Replace 'BlynkSimpleEsp8266' with 'BlynkSimpleEsp32.h. for ESP32
WidgetTerminal terminal(V40);			// Attach virtual serial terminal to Virtual Pin V40
#include <SimpleTimer.h>				// https://github.com/jfturcot/SimpleTimer
#include <TimeLib.h>
#include <WidgetRTC.h>
SimpleTimer Timer;
WidgetRTC rtc;							//Inicjacja widgetu zegara czasu rzeczywistego RTC

static volatile int timerID=-1;			//Przetrzymuje ID Timera https://desire.giesecke.tk/index.php/2018/01/30/change-global-variables-from-isr/
int timerIDReset=-1;					//Przetrzymuje ID Timera https://desire.giesecke.tk/index.php/2018/01/30/change-global-variables-from-isr/

int		Tryb_Sterownika		= 0;		// Tryb_Sterownika 0 = AUTO, 1 = ON, 2 = OFF, 3 = MANUAL
float	SetHumidManual		= 75;		// Wilgotności przy której załączy się wentylator w trybie manualnym
float	SetHumidActual		= 50;		// Wilgotności przy której załączy się wentylator
float	RoomHumid			= 0;		// Wilgotności w pokoju, potrzebna do wyznaczenia wartości wilgotności przy której ma się załączyć wentylator
int		PhotoResValue		= 0;		// Store value from photoresistor (0-1023)
int		ProgPhotoresistor	= 300;		// Próg jasności od którego zacznie działać iluminacja sedesu (nie powinno podświetlać jeśli światło w łazience zapalone)
static volatile boolean	isLED_Light	= false;	// TRUE jeśli diody świecą FALSE jeśli nie świecą
boolean		HeatCO			= false;	// informacja wysyłana na V18 TRUE jeśli piec grzeje i FALSE jeśli nie grzeje (funkcja Bridge)
boolean		RestartESP		= false;	// Gdy true i w terminalu YES wykona restart
float temp(NAN), hum(NAN), pres(NAN), dewPoint(NAN), absHum(NAN), heatIndex(NAN);

//STAŁE
//const char	ssid[]		= "Your SSID";							// Not required with AutoConnect
//const char	pass[]		= "Router password";					// Not required with AutoConnect
const char	auth[]			= "q3lvQ_5iEAtG06rXBrmUtsPZ_2DhuGQj";	// Token Łazienka Rymanowska
const char	server[] 		= "192.168.1.204";  					// IP for your Local Server or DNS server addres stecko.duckdns.org
const int	port 			= 8080;									// Port na którym jest serwer Blykn
const int	BathFan			= D5;		// Deklaracja pinu na który zostanie wysłany sygnał załączenia wentylatora
const int	Piec_CO			= D6;		// Deklaracja pinu na którym będzie włączany piec CO
const int	PIR_Sensor		= D7;		// Deklaracja pinu z sensorem ruchu AM312
const int	LED_Light		= D8;		// Deklaracja pinu z MOSFETem do iluminacji sedesu
const int	PhotoResistor	= A0;		// Deklaracja pinu z sensorem światła
const float	HumidHist		= 4;		// Wartość histerezy dla wilgotności

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
			Serial.println("Blynk.connect() START");
			Blynk.connect();
			Serial.println("Blynk.connect() STOP");
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
			// Port defaults to 8266
			ArduinoOTA.setPort(8266);

			// Hostname defaults to esp8266-[ChipID]
			ArduinoOTA.setHostname("Lazienka_Rymanowska");

			// No authentication by default
			// ArduinoOTA.setPassword("admin");

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

//Załączanie wentylatora w łazience jeśli warunek spełnionyBathFan_Value
void Bathrum_Humidity_Control()
{
	if (Tryb_Sterownika == 2)		//Wilgotność w trybie ręcznym OFF
	{
		digitalWrite(BathFan, HIGH);		//turn on relay with voltage HIGH
		Blynk.virtualWrite(V8, 0);		//Wentylator Wyłączony
	}
	else if (Tryb_Sterownika == 1 && HeatCO == false)			//Wilgotność w trybie ręcznym ON
	{
		digitalWrite(BathFan, LOW);		//turn on relay with voltage LOW
		Blynk.virtualWrite(V8, 255);		//Wentylator włączony
	}
	else if (hum >= SetHumidActual + HumidHist && isLED_Light == false && HeatCO == false)
	{
		if (temp > 20 || analogRead(PhotoResistor) < ProgPhotoresistor)
		{
			digitalWrite(BathFan, LOW);	//turn on relay with voltage HIGH
			Blynk.virtualWrite(V8, 255);	//Wentylator Włączony
		}
		else
		{
			digitalWrite(BathFan, HIGH);	//turn on relay with voltage HIGH
			Blynk.virtualWrite(V8, 0);	//Wentylator Wyłączony
		}
	}
	else if (hum <= SetHumidActual - HumidHist || HeatCO == true)
	{
		digitalWrite(BathFan, HIGH);		//turn on relay with voltage HIGH
		Blynk.virtualWrite(V8, 0);		//Wentylator Wyłączony
	}
}

//Odczyt wskazań z czujnika BME280
void Read_BME280_Values()
{
	BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
	BME280::PresUnit presUnit(BME280::PresUnit_hPa);

	bme.read(pres, temp, hum, tempUnit, presUnit);
	temp = temp -0.33;		//Korekta dla temperatury. BME280 się trochę grzeje 
	pres = pres + 24.634;		//Korekta dostosowująca do ciśnienia na poziomie morza
	hum  = hum - 8;			//Korekta poziomu wilgotności odczytanegoe prze BME280.

	EnvironmentCalculations::TempUnit     envTempUnit =  EnvironmentCalculations::TempUnit_Celsius;
	//Dane obliczane na podstawie danych z czujnika
	dewPoint = EnvironmentCalculations::DewPoint(temp, hum, envTempUnit);
	absHum = EnvironmentCalculations::AbsoluteHumidity(temp, hum, envTempUnit);
	heatIndex = EnvironmentCalculations::HeatIndex(temp, hum, envTempUnit);
}

//Zwraca siłę sygnału WiFi sieci do której jest podłączony w %. REF: https://www.adriangranados.com/blog/dbm-to-percent-conversion
int WiFi_Strength (long Signal)
{
	return constrain(round((-0.0154*Signal*Signal)-(0.3794*Signal)+98.182), 0, 100);
}

//signal strength levels https://www.netspotapp.com/what-is-rssi-level.html
String WiFi_levels(long Signal)
{
	String WiFiStrength = "WiFi Strenght ?";

	if (Signal >= -50)
	{
		WiFiStrength = "Excellent";
	}
	else if (Signal < -50 && Signal >= -60)
	{
		WiFiStrength = "Very good";
	}
	else if (Signal < -60 && Signal >= -70)
	{
		WiFiStrength = "Good";
	}
	else if (Signal < -70 && Signal >= -80)
	{
		WiFiStrength = "Low";
	}
	else if (Signal < -80 && Signal >= -90)
	{
		WiFiStrength = "Very low";
	}
	else if (Signal < -90 )
	{
		WiFiStrength = "Unusable";
	}
	
	return WiFiStrength;
}

//Wysyła dane na serwer Blynk
void Wyslij_Dane()
{
	Blynk.virtualWrite(V0, temp);							// Temperatura [°C]
	Blynk.virtualWrite(V1, hum);							// Wilgotność [%]
	Blynk.virtualWrite(V2, pres);							// Ciśnienie [hPa]
	Blynk.virtualWrite(V3, dewPoint);						// Temperatura punktu rosy [°C]
	Blynk.virtualWrite(V4, absHum);							// Wilgotność bezwzględna [g/m³]
	Blynk.virtualWrite(V5, heatIndex);						// Temperatura odczuwalna [°C]
	Blynk.virtualWrite(V6, SetHumidActual);					// Wilgotności przy której załączy się wentylator w trybie automatycznym [%] 
	Blynk.virtualWrite(V25, WiFi_Strength(WiFi.RSSI())); 	// Siła sygnału Wi-Fi [%], constrain() limits range of sensor values to between 0 and 100
	Blynk.virtualWrite(V55, analogRead(PhotoResistor));		// Czujnik światła
	Blynk.virtualWrite(V56, digitalRead(PIR_Sensor));		// Czujnik ruchu
}

//Ustawienie trybów sterowania wilgotnością
void TrybManAuto()
{
	switch (Tryb_Sterownika)
	{
		case 0:					//Wilgotność w trybie AUTO na podstawie wilgotności w pokoju +- histereza
			if (RoomHumid + 15 < 50)	//Wilgotność w trybie AUTO
				{
					SetHumidActual = 50;
				}
			else
			{
				SetHumidActual = RoomHumid + 15;
			}
			break;
		case 1:					//Wilgotność w trybie ręcznym ON
			SetHumidActual = hum;
			break;
		case 2:					//Wilgotność w trybie ręcznym OFF
			SetHumidActual = 0;
			break;
		case 3:					//Wilgotność w trybie MANUAL z zadaną wilgotnością
			SetHumidActual = SetHumidManual;
			break;
		default:				//Wartość domyślna AUTO
			
			SetHumidActual = RoomHumid + 15;
			
			break;
	}
}

//Wyłącza możliwość restartu ESP (timer ustawiony na 30s)
void RestartCounter()
{	
	terminal.clear();
	terminal.println("30s past. Try again.");
	terminal.flush();
	RestartESP = false;	//Gdy true i w terminalu YES wykona restart
}

//Soft restart sterownika
void RestartESP32()
{
	terminal.clear();
	terminal.println("Restarting!");
	terminal.flush();
	//delay(1000);
	ESP.restart(); 	//Restartuje sterownik
}

//Obsługa terminala
BLYNK_WRITE(V40)
{
	String TerminalCommand = param.asStr();
	TerminalCommand.toLowerCase();

	if (String("ports") == TerminalCommand)
	{
		terminal.clear();
		terminal.println("PORT     DESCRIPTION        UNIT");
		terminal.println("V0   ->  Temperature        °C");
		terminal.println("V1   ->  Humidity           %");
		terminal.println("V2   ->  Pressure           HPa");
		terminal.println("V3   ->  DewPoint           °C");
		terminal.print("V4   ->  Abs Humidity       g/m");
		terminal.println("\xc2\xb3");	//^3 potęga 3 https://www.utf8-chartable.de/unicode-utf8-table.pl?start=128&number=128&utf8=string-literal&unicodeinhtml=hex
		terminal.println("V5   ->  Heat Index         °C");
		terminal.println("V10  <-  SetHumidManual     %");
		terminal.println("V11  <-  Fan Manual & State 1,2,3,4");
		terminal.println("V18  <-  HeatCO             0/1");
		terminal.println("V25  ->  WiFi Signal        %");
		terminal.println("V55  ->  PhotoResistor      -");
		terminal.println("V56  ->  PIR_Sensor         0/1");
		terminal.println("V40 <->  Terminal           String");
	}
	else if (String("values") == TerminalCommand)
	{
		terminal.clear();
	      terminal.println("PORT   DATA             VALUE");
		terminal.print("V0     Temperature    = ");
		terminal.print(temp,2);
		terminal.println("°C");
		terminal.print("V1     Humidity       = ");
		terminal.print(hum);
		terminal.println("%");
		terminal.print("V2     Pressure       = ");
		terminal.print(pres);
		terminal.println("HPa");
		terminal.print("V3     DewPoint       = ");
		terminal.print(dewPoint);
		terminal.println("°C");
		terminal.print("V4     Abs Humidity   = ");
		terminal.print(absHum);
		terminal.print("g/m");
		terminal.println("\xc2\xb3");	//^3 potęga 3 https://www.utf8-chartable.de/unicode-utf8-table.pl?start=128&number=128&utf8=string-literal&unicodeinhtml=hex
		terminal.print("V5     Heat Index     = ");
		terminal.print(heatIndex);
		terminal.println("°C");
		terminal.print("V10    SetHumidManual = ");
		terminal.println(SetHumidManual);
		terminal.print("V18    HeatCO         = ");
		terminal.println(HeatCO);
		terminal.print("V55    PhotoResistor  = ");
		terminal.println(analogRead(PhotoResistor));
		terminal.print("V56    PIR_Sensor     = ");
		terminal.println(digitalRead(PIR_Sensor));
		terminal.print("V25    WiFi Signal    = ");
		terminal.print(WiFi_Strength(WiFi.RSSI()));
		terminal.print("%, ");
		terminal.print(WiFi.RSSI());
		terminal.print("dBm, ");
		terminal.println(WiFi_levels(WiFi.RSSI()));
		terminal.print("AutoConnect IP        = ");
		terminal.print(WiFi.localIP().toString() + "/_ac");
	}
	else if (String("restart") == TerminalCommand)
	{
		terminal.clear();
		terminal.println("Are you sure you want to restart?");
		terminal.println("Type YES if you are sure to restart...");
		terminal.flush();
		RestartESP = true;	//Gdy true i w terminalu YES wykona restart
		Timer.setTimeout(30000, RestartCounter);
	}
	else if (String("yes") == TerminalCommand)
	{	
		if (RestartESP)
		{
			terminal.clear();
			terminal.println("To be restarted in 3...2...1...");
			terminal.flush();
			Timer.setTimeout(2000, RestartESP32);	//Zrestartuje sterownik za 1s
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
		terminal.println("Type 'RESTART' to restart.");
		terminal.println("Type 'CLS' to clear terminal");
		terminal.println("or 'HELLO' to say hello!");
	}
	// Ensure everything is sent
	terminal.flush();
}

//Ustawienie progu wilgotności powyżej którego włączy się wentylator (plus próg)
BLYNK_WRITE(V10)
{
	SetHumidManual = param.asInt();
	Bathrum_Humidity_Control();	//Uruchomienie funkcji Bathrum_Humidity_Control() aby zadziałało natychmiast
}

//Informacja czy piec grzeje
BLYNK_WRITE(V18)
{
    HeatCO = param.asFloat(); 		//pinData variable will store value that came via Bridge
}

//Wilgotność w pokoju, przesyłana z Wemos D1
BLYNK_WRITE(V21)
{
	RoomHumid = param.asInt();
}

//Tryb_Sterownika 0 = AUTO, 1 = ON, 2 = OFF, 3 = MANUAL
BLYNK_WRITE(V11)
{
	switch (param.asInt())
	{
		case 1:				//AUTO
			Tryb_Sterownika = 0;
			Blynk.setProperty(V10,"color","#163C49"); //Zdławiony-niebieski
			break;
		case 2:				//ON
			Tryb_Sterownika = 1;
			Blynk.setProperty(V10,"color","#163C49"); //Zdławiony-niebieski
			break;
		case 3:				//OFF
			Tryb_Sterownika = 2;
			Blynk.setProperty(V10,"color","#163C49"); //Jasno-niebieski
			break;
		case 4:				//MAN
			Tryb_Sterownika = 3;
			Blynk.setProperty(V10,"color","#04B0E2"); //Zdławiony-niebieski
			break;
		default:		//Wartość domyślna AUTO
			Tryb_Sterownika = 0;
			Blynk.setProperty(V10,"color","#163C49"); //Zdławiony-niebieski
			break;
	}
	Bathrum_Humidity_Control();	//Uruchomienie funkcji Bathrum_Humidity_Control() aby zadziałało natychmiast
}

//Wyłączenie podświetlania sedesu
void SedesIlluminationOFF()
{
	digitalWrite(LED_Light, LOW);
	isLED_Light = false;
}

//Obsługa przerwań wywoływanych przez czujnik PIR AM312
//ICACHE_RAM_ATTR void handleInterrupt()
IRAM_ATTR void handleInterrupt()
{
	if ( isLED_Light && Timer.isEnabled(timerID) && analogRead(PhotoResistor) < ProgPhotoresistor)
	{
		Timer.restartTimer(timerID);				//Wydłuża iluminacje sedesu o kolejne 30s
	}
	else if (analogRead(PhotoResistor) < ProgPhotoresistor )	//Returns true if the specified timer is enabled
	{
		timerID = Timer.setTimeout(45000, SedesIlluminationOFF);//Wyłączy iluminacje sedesu za 45s
		digitalWrite(LED_Light, HIGH);
		isLED_Light = true;
	}
}

//Uruchamia po kolei wszystkie niezbędne funcje
void MainFunction()
{
	Read_BME280_Values();			//Odczyt danych z czujnika BME280
	TrybManAuto();				//Ustawienie trybów sterowania wilgotnością
	Bathrum_Humidity_Control();		//Włącza wentylator jeśli wilgotność przekracza próg ale Piec CO jest wyłączony
	Wyslij_Dane();				//Wysyła dane do serwera Blynk

	if (digitalRead(PIR_Sensor) == 1)	//Ma na celu przedłużenie timera do podświetlenia
	{
		handleInterrupt();
	}
}

//---------------------------------------------------------------------------------------------------------------------------------------------

void setup()
{
	Serial.begin(115200);
	// Autoconnect
	Config.hostName = "Lazienka";		// Sets host name to SotAp identification
	Config.homeUri = "/_ac";		// Sets home path of Sketch application
	Config.retainPortal = true;		// Launch the captive portal on-demand at losing WiFi
	Config.autoReconnect = true;		// Enable auto-reconnect
	Config.ota = AC_OTA_BUILTIN;
	Portal.config(Config);    		// Don't forget it.
	if (Portal.begin())
	{	
		Serial.println("WiFi connected: " + WiFi.localIP().toString());
		//Server.on("/", handleRoot);
	}

	//WiFi.begin(ssid, pass);
	Blynk.config(auth, server, port);   // for local servernon-blocking, even if no server connection
	//Blynk.config(auth);				//For cloud




	//Inicjalizacja Timerów
	Timer.setInterval(30000, blynkCheck);		//Sprawdza czy BLYNK połączony co 30s
	Timer.setInterval(3000, MainFunction);		//Uruchamia wszystko w pętli co 3s

	//Ustawianie pinów
	pinMode(BathFan, OUTPUT);			//Deklaracja pinu na który zostanie wysłany sygnał załączenia wentylatora
	digitalWrite(BathFan, HIGH);			//Domyślnie wyłączony (stan wysoki HIGH)
	pinMode(Piec_CO, OUTPUT);			//Deklaracja pinu na którym będzie włączany piec CO
	digitalWrite(Piec_CO, HIGH);			//Domyślnie wyłączony (stan wysoki HIGH)

	pinMode(PIR_Sensor, INPUT);			//Deklaracja pinu z sensorem ruchu AM312

	pinMode(LED_Light, OUTPUT);			//Deklaracja pinu z MOSFETem do iluminacji sedesu
	digitalWrite(LED_Light, LOW);			//Domyślnie wyłączony (stan niski LOW)

	pinMode(PhotoResistor, INPUT);			//Set pResistor - A0 pin as an input (optional)
	attachInterrupt(digitalPinToInterrupt(PIR_Sensor), handleInterrupt, HIGH);	//Obsługa przerwań dla czujnika ruchu

	//inicjowanie czujnika BME280
	Wire.begin();
	if (!bme.begin())
	{
		Serial.println("Could not find a valid BME280 sensor, check wiring!");
		while (1);
	}
	switch(bme.chipModel())
	{
	case BME280::ChipModel_BME280:
		Serial.println("Found BME280 sensor! Success.");
		break;
	case BME280::ChipModel_BMP280:
		Serial.println("Found BMP280 sensor! No Humidity available.");
		break;
	default:
		Serial.println("Found UNKNOWN sensor! Error!");
	}

	blynkCheck(); 					//Piewsze połaczenie z Blynk, nie trzeba czekać 30s po restarcie
}

void loop()
{	
	Timer.run();
	if (WiFi.status() == WL_CONNECTED)
	{
		// Here to do when WiFi is connected.
		if (Blynk.connected()) Blynk.run();
		//OTA_Handle();			//Obsługa OTA (Over The Air) wgrywanie nowego kodu przez Wi-Fi
		if(Timer.isEnabled(timerIDReset))
		{
			Timer.deleteTimer(timerIDReset);
		}
	}
	 else	//Zrestartuje sterownik jeśli brak sieci przez 5min
	{
		if (Timer.isEnabled(timerIDReset) == false)
		{
			timerIDReset = Timer.setTimeout(300000, RestartESP32);
		}
	}
	
	Portal.handleClient();

	if ( isLED_Light && analogRead(PhotoResistor) > ProgPhotoresistor )
	{
		Timer.deleteTimer(timerID);	//Wyłącza Timer
		SedesIlluminationOFF();		//Wyłączenie podświetlania sedesu
	}
}
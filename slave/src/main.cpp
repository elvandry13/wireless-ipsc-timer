#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define START_PIN D5
#define RESET_PIN D6
#define BUZZER_PIN D7
#define LED_PIN D2

unsigned long mulai, selesai, dataStopWatch;
int i = 0;
int fPaus = 0;
long lastButtonStart = 0; 
long lastButtonStop = 0; 
unsigned long delayAntiBouncing = 50; 
long dataPaus = 0;

uint8_t stopTrigger = 0;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

void OnDataRecv(uint8_t *mac_addr, uint8_t *incomingData, uint8_t len)
{
	char macStr[18];
	snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
			 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

	Serial.print("Last Packet Recv from: ");
	Serial.println(macStr);
	Serial.print("Last Packet Recv Data: ");
	Serial.println(*incomingData);
	Serial.println("");

	stopTrigger = *incomingData;
}

void setup()
{
	// Initialize Serial Monitor
	Serial.begin(115200);

	// LCD init
	lcd.begin(16, 2);
	lcd.setCursor(0, 0);
	lcd.print("    Wireless    ");
	lcd.setCursor(0, 1);
	lcd.print("   IPSC Timer   ");
	delay(2000);
	lcd.clear();
	lcd.print("Press Start");
	lcd.setCursor(0, 1);
	lcd.print("0:0:0.0");

	// Set device as a Wi-Fi Station
	WiFi.mode(WIFI_STA);

	// Init ESP-NOW
	if (esp_now_init() != 0)
	{
		Serial.println("Error initializing ESP-NOW");
		return;
	}

	// Once ESPNow is successfully Init, we will register for recv CB to
	// get recv packer info
	esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
	esp_now_register_recv_cb(OnDataRecv);

	pinMode(BUZZER_PIN, OUTPUT);
	pinMode(LED_PIN, OUTPUT);
	pinMode(START_PIN, INPUT);
	pinMode(RESET_PIN, INPUT);
	digitalWrite(START_PIN, LOW);
	digitalWrite(RESET_PIN, LOW);
	digitalWrite(LED_PIN, LOW);
}

void loop()
{
	// Start timer
	if (digitalRead(START_PIN) == 1)
	{
		if ((millis() - lastButtonStart) > delayAntiBouncing)
		{
			if (i == 0)
			{
				digitalWrite(BUZZER_PIN, HIGH);
				delay(100);
				digitalWrite(BUZZER_PIN, LOW);
				lcd.clear();
				lcd.setCursor(0, 0);
				lcd.print("Start Timer");
				mulai = millis();
				fPaus = 0;
				i = !i;
			}
		}
		lastButtonStart = millis();
	}

	// Stop timer
	else if (stopTrigger == 1)
	{
		if (i == 1)
		{
			lcd.setCursor(0, 0);
			lcd.print("Stop Timer  ");
			dataPaus = dataStopWatch;
			fPaus = 1;
			i = !i;
			digitalWrite(LED_PIN, HIGH);
			digitalWrite(BUZZER_PIN, HIGH);
			delay(2000);
			digitalWrite(BUZZER_PIN, LOW);
		}
		digitalWrite(LED_PIN, HIGH);
	}

	else if (stopTrigger == 0)
	{
		digitalWrite(LED_PIN, LOW);
	}
	
	
	// Reset timer
	if (digitalRead(RESET_PIN) == 1 && fPaus == 1)
	{
		digitalWrite(LED_PIN, LOW);		
		dataStopWatch = 0;
		dataPaus = 0;
		lcd.clear();
		lcd.print("Reset Stopwatch");
		lcd.setCursor(0, 1);
		lcd.print("0:0:0.0");
		for (int i = 0; i < 2; i++)
		{
			digitalWrite(BUZZER_PIN, HIGH);
			delay(250);
			digitalWrite(BUZZER_PIN, LOW);
			delay(250);
		}
		delay(2000);
		lcd.clear();
		lcd.print("Press Start");
		lcd.setCursor(0, 1);
		lcd.print("0:0:0.0");
	}

	if (i == 1)
	{
		selesai = millis();
		float jam, menit, detik, miliDetik;
		unsigned long over;

		// MATH time!!!
		dataStopWatch = selesai - mulai;
		dataStopWatch = dataPaus + dataStopWatch;

		jam = int(dataStopWatch / 3600000);
		over = dataStopWatch % 3600000;
		menit = int(over / 60000);
		over = over % 60000;
		detik = int(over / 1000);
		miliDetik = over % 1000;

		lcd.setCursor(0, 1);
		lcd.print(jam, 0);
		lcd.print(":");
		lcd.print(menit, 0);
		lcd.print(":");
		lcd.print(detik, 0);
		lcd.print(".");
		if (jam < 10)
		{
			lcd.print(miliDetik, 0);
			lcd.print("   ");
		}
	}
}

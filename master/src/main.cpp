#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

#define SENSOR_GETAR D0

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xEC, 0xFA, 0xBC, 0x5C, 0x6A, 0x6F};
uint8_t myData;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
	Serial.print("Last Packet Send Status: ");
	if (sendStatus == 0)
	{
		Serial.println("Delivery success");
	}
	else
	{
		Serial.println("Delivery fail");
	}
}

void setup()
{
	// Init Serial Monitor
	Serial.begin(115200);

	// Set device as a Wi-Fi Station
	WiFi.mode(WIFI_STA);

	// Init ESP-NOW
	if (esp_now_init() != 0)
	{
		Serial.println("Error initializing ESP-NOW");
		return;
	}

	// Once ESPNow is successfully Init, we will register for Send CB to
	// get the status of Trasnmitted packet
	esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
	esp_now_register_send_cb(OnDataSent);

	// Register peer
	esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

	pinMode(SENSOR_GETAR, INPUT);
}

void loop()
{
	if (digitalRead(SENSOR_GETAR) == 1)
	{
		myData = 1;
		esp_now_send(broadcastAddress, &myData, sizeof(myData));
		Serial.println("getar");
	}
	else
	{
		myData = 0;
		esp_now_send(broadcastAddress, &myData, sizeof(myData));
	}
	delay(50);
}

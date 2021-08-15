#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "telegramcomms.h"

//Replace with your network credentials
const char* ssid = "HappyWIFI-EXT";
const char* password = "H1234567i!";

bool WiFiInit()
{
// Wi-Fi connection
  WiFi.begin(ssid, password);
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  return true;
}